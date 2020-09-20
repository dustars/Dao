#include "Renderer.h"
#include <stb_image_write.h>

Renderer::Renderer(Window& parent)
	: RenderBase(parent),
	light1(new PointLight(Vector3(2000.f, 600.f, 2000.f), Vector4(1.0f, 0.9f, 0.9f, 1.f))),
	renderFBO(new FrameBuffer(width, height)),
	textRenderer(TextRenderer(width, height))
{
	//Pre-defined values to take screenshot at the same position and orientaion.
	//camera = new Camera(parent, 12.78f, 172.82f, Vector3(0.f, 250.f, 5.f));
	camera = new Camera(parent, 0, 0, Vector3(0.f, 0.f, 0.f));
	projMatrix = Matrix4::Perspective(1.0f, 20000.0f, (float)width / (float)height, 45.0f);

#ifdef TESTING_OBJECT
	object = new RenderObject();
	if (!object->SetShader("shader/GeneralVS.glsl", "shader/GeneralFS.glsl")) {
		cout << "Shader set up failed!" << endl;
	}
	if (!object->GetTexture()->SetTexture("../assets/Textures/container.jpg")) {
		cout << "Texture set up failed!" << endl;
	}
	object->GetMesh()->CreatePlane();
#else
	object = new RenderObject();
	if (!object->SetShader("shader/GeneralVS.glsl", "shader/GeneralFS.glsl")) {
		cout << "Shader set up failed!" << endl;
	}
	if (!object->GetTexture()->SetTexture("../assets/Textures/Barren Reds.jpg")) {
		cout << "Texture set up failed!" << endl;
	}
	object->SetMesh(new HeightMap(5, 2, 0.707, MAPWIDTH, MAPLENGTH));
#endif

#ifdef ATMOSPHERE
	CreateAtmosphericScatteringModel();
#else
	CreateSkybox();
#endif // ATMOSPHERE

#ifdef RENDER_CLOUD
	CreateCloud();
#endif // RENDER_CLOUD

#ifdef SHADOW_MAPPING
	CreateShadowMap();
#endif // SHADOW_MAPPING

#ifdef IMGUI
	ImGUIInit(parent);
#endif // IMGUI
	init = true;
}

Renderer::~Renderer()
{
	if (camera)						delete camera;
	if (object)						delete object;
	if (trajectory)					delete trajectory;
	if (skybox)						delete skybox;
	if (particleMaster)				delete particleMaster;
#ifdef IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

void Renderer::Update(float dt)
{

	camera->UpdateCamera(dt);
	UpdateControl(dt);

	//Temporary method to limit frame rate at 60FPS
	oneFramePerMilliSecond += dt;
	if (oneFramePerMilliSecond > renderFrames) {
		//Update
		if (particleMaster) particleMaster->Update(oneFramePerMilliSecond);
		if (trajectory) trajectory->GetMesh()->Update(oneFramePerMilliSecond);
		UtilityUpdate();

		//Render
		Render();
	}
}

void Renderer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#ifdef RENDER_CLOUD
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO->GetFrameBuffer());
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
#endif

#ifdef ATMOSPHERE
	if (atmosphereScattering.get()) RenderAtmosphericScatteringModel();
#else
	if (skybox) renderSkyBox();
#endif // ATMOSPHERE

#ifdef RENDER_CLOUD
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	RenderCloud();
#endif

#ifdef SHADOW_MAPPING
	RenderShadowMap();
#endif

#ifdef TESTING_OBJECT
	if (object) renderObject();
#else
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	if (object) renderObject();
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
#endif // TESTING_OBJECT

#ifdef OFFLINE
	ScreenShot("Offline_Rendering");
#else
	UtilityRender();
	SwapBuffers(deviceContext);
#endif
}

void Renderer::UtilityUpdate()
{
	oneFramePerSecond += oneFramePerMilliSecond;
	frameCount++;
	oneFramePerMilliSecond = 0;
	if (oneFramePerSecond > 1000.f) {
		oneFramePerSecond = 0;
		fps = frameCount;
		frameCount = 0;
	}
}

void Renderer::UtilityRender()
{
#ifdef IMGUI
	if (camera->GetShowGUI()) RenderImGUI();
#endif // IMGUI
	if (isRenderingText) RenderText();
}

void Renderer::renderObject()
{
	glUseProgram(object->GetShader()->GetProgram());

#ifdef SHADOW_MAPPING
	glBindTextureUnit(2, shadowFBO->GetDepthTexture());
	glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "lightMatrix"), 1, GL_FALSE, (float*)&lightMatrix);
#endif

	glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "ModelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "ViewMatrix"), 1, GL_FALSE, (float*)&camera->BuildViewMatrix());
	glUniformMatrix4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "ProjMatrix"), 1, GL_FALSE, (float*)&projMatrix);
	if (light1.get()) {
		glUniform3fv(glGetUniformLocation(object->GetShader()->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
		glUniform3fv(glGetUniformLocation(object->GetShader()->GetProgram(), "sunDir"), 1, (float*)&camera->GetSunDirection());
		glUniform4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "LightPos"), 1, (float*)&light1->GetPosition());
		glUniform4fv(glGetUniformLocation(object->GetShader()->GetProgram(), "LightColor"), 1, (float*)&light1->GetColor());
	}

	object->Draw();
	glUseProgram(0);
}

void Renderer::renderSkyBox()
{
	glDepthMask(GL_FALSE);
	glUseProgram(skybox->GetShader()->GetProgram());
	glUniformMatrix4fv(glGetUniformLocation(skybox->GetShader()->GetProgram(), "ViewMatrix"), 1, GL_FALSE,(float*)&camera->BuildViewMatrix());
	glUniformMatrix4fv(glGetUniformLocation(skybox->GetShader()->GetProgram(), "ProjMatrix"), 1, GL_FALSE, (float*)&projMatrix);
	
	skybox->GetMesh()->Draw();

	glUseProgram(0);
	glDepthMask(GL_TRUE);
}

void Renderer::RenderText()
{
	textRenderer.RenderText("FPS: " + to_string(fps), 10.0f, height - 25.f);
	string CameraPositionText = "Camera Position: " +
		to_string(camera->GetPosition().x) + ", " +
		to_string(camera->GetPosition().y) + ", " +
		to_string(camera->GetPosition().z);
	textRenderer.RenderText(CameraPositionText, 10.0f, height - 45.f);
	textRenderer.RenderText("Camera Pitch: " + to_string(camera->GetPitch()), 10.0f, height - 65.f);
	textRenderer.RenderText("Camera Yaw: " + to_string(camera->GetYaw()), 10.0f, height - 85.f);
	textRenderer.RenderText("Sun Zenith : " + to_string(camera->GetSunZenithDegree()), 10.0f, height - 105.f);
	textRenderer.RenderText("Sun Azimuth: " + to_string(camera->GetSunAzimuthDegree()), 10.0f, height - 125.f);
}

void Renderer::CreateCloud()
{
	if (!cloudShader.SetShader("shader/CloudVShader.glsl", "shader/CloudFShader.glsl")) {
		cout << "Shader set up failed!" << endl;
	}
	cloudShader.GetMesh()->CreatePlane(); //ray marching plane

	cloudModel.reset(new atmosphere::Cloud(128));

	glUseProgram(cloudShader.GetShader()->GetProgram());
	glBindTextureUnit(2, cloudModel->GetBaseShapeTex());
	glBindTextureUnit(3, cloudModel->GetDetailShapeNoiseTex());
	glBindTextureUnit(4, cloudModel->GetWeatherMapTex());
	glBindTextureUnit(5, cloudModel->GetBlueNoiseTex());

	glUniformMatrix4fv(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "projMatrix"), 1, GL_FALSE, (float*)&projMatrix);

	glUniform1f(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "cloudLayerRadius"), cloudModel->cloudLayerRadius);
	glUniform1f(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "cloudHeightAboveGround"), cloudModel->cloudHeightAboveGround);
	glUniform1f(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "cloudLayerLength"), cloudModel->cloudLayerLength);

	glUseProgram(0);
}

void Renderer::RenderCloud()
{
	glUseProgram(cloudShader.GetShader()->GetProgram());

	glBindTextureUnit(0, renderFBO->GetColorTexture());

	glUniformMatrix4fv(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "viewMatrix"), 1, true, (float*)&camera->BuildViewMatrix());
	glUniform3fv(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());;
	glUniform3fv(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "sunDirection"), 1, (float*)&camera->GetSunDirection());
	//Control parameters
	glUniform1f(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "globalCoverage"), cloudModel->globalCoverage);
	glUniform1f(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "globalDensity"), cloudModel->globalDensity);
	glUniform1f(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "cloudScale"), cloudModel->cloudScale);
	glUniform1f(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "cloudOffset"), cloudModel->cloudOffset);
	glUniform1i(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "sampleSteps"), cloudModel->sampleSteps);
	glUniform1i(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "lightSampleSteps"), cloudModel->lightSampleSteps);
	glUniform1f(glGetUniformLocation(cloudShader.GetShader()->GetProgram(), "lightAbsorptionFactor"), cloudModel->lightAbsorptionFactor);
	
	cloudShader.Draw();
	glUseProgram(0);
}

void Renderer::CreateAtmosphericScatteringModel()
{
	//These are all parameter needed for atmospheric scattering model.
	enum Luminance { NONE, APPROXIMATE, PRECOMPUTED };
	constexpr double kPi = 3.1415926;
	constexpr double kSunAngularRadius = 0.00935 / 2.0;
	constexpr double kSunSolidAngle = kPi * kSunAngularRadius * kSunAngularRadius;
	constexpr double kLengthUnitInMeters = 1000.0;

	bool	use_constant_solar_spectrum_ = false;
	bool	use_ozone_ = true;
	bool	use_combined_textures_ = true;
	bool	use_half_precision_ = true;
	bool	use_luminance_ = APPROXIMATE;
	bool	do_white_balance_ = false;

	constexpr int kLambdaMin = 360;
	constexpr int kLambdaMax = 830;
	constexpr double kSolarIrradiance[48] = {
	  1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887, 1.61253,
	  1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
	  1.8685, 1.8931, 1.85149, 1.8504, 1.8341, 1.8345, 1.8147, 1.78158, 1.7533,
	  1.6965, 1.68194, 1.64654, 1.6048, 1.52143, 1.55622, 1.5113, 1.474, 1.4482,
	  1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758, 1.2367, 1.2082,
	  1.18737, 1.14683, 1.12362, 1.1058, 1.07124, 1.04992
	};

	constexpr double kOzoneCrossSection[48] = {
	  1.18e-27, 2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27, 5.52e-27,
	  8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26, 7.752e-26, 9.016e-26,
	  1.48e-25, 1.602e-25, 2.139e-25, 2.755e-25, 3.091e-25, 3.5e-25, 4.266e-25,
	  4.672e-25, 4.398e-25, 4.701e-25, 5.019e-25, 4.305e-25, 3.74e-25, 3.215e-25,
	  2.662e-25, 2.238e-25, 1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26,
	  6.566e-26, 5.105e-26, 4.15e-26, 4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
	  2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27
	};

	constexpr double kDobsonUnit = 2.687e20;
	constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;
	constexpr double kConstantSolarIrradiance = 1.5;
	constexpr double kBottomRadius = 6360000.0;
	constexpr double kTopRadius = 6420000.0;
	constexpr double kRayleigh = 1.24062e-6;
	constexpr double kRayleighScaleHeight = 8000.0;
	constexpr double kMieScaleHeight = 1200.0;
	constexpr double kMieAngstromAlpha = 0.0;
	constexpr double kMieAngstromBeta = 5.328e-3;
	constexpr double kMieSingleScatteringAlbedo = 0.9;
	constexpr double kMiePhaseFunctionG = 0.8;
	constexpr double kGroundAlbedo = 0.1;
	const double max_sun_zenith_angle =
		(use_half_precision_ ? 102.0 : 120.0) / 180.0 * kPi;

	atmosphere::DensityProfileLayer
		rayleigh_layer(0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
	atmosphere::DensityProfileLayer mie_layer(0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0);
	std::vector<atmosphere::DensityProfileLayer> ozone_density;
	ozone_density.push_back(
		atmosphere::DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
	ozone_density.push_back(
		atmosphere::DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));

	std::vector<double> wavelengths;
	std::vector<double> solar_irradiance;
	std::vector<double> rayleigh_scattering;
	std::vector<double> mie_scattering;
	std::vector<double> mie_extinction;
	std::vector<double> absorption_extinction;
	std::vector<double> ground_albedo;
	for (int l = kLambdaMin; l <= kLambdaMax; l += 10) {
		double lambda = static_cast<double>(l) * 1e-3;
		double mie =
			kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
		wavelengths.push_back(l);
		if (use_constant_solar_spectrum_) {
			solar_irradiance.push_back(kConstantSolarIrradiance);
		}
		else {
			solar_irradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
		}
		rayleigh_scattering.push_back(kRayleigh * pow(lambda, -4));
		mie_scattering.push_back(mie * kMieSingleScatteringAlbedo);
		mie_extinction.push_back(mie);
		absorption_extinction.push_back(use_ozone_ ?
			kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10] :
			0.0);
		ground_albedo.push_back(kGroundAlbedo);
	}

	atmosphereScattering.reset(new atmosphere::Model(wavelengths, solar_irradiance, kSunAngularRadius,
		kBottomRadius, kTopRadius, { rayleigh_layer }, rayleigh_scattering,
		{ mie_layer }, mie_scattering, mie_extinction, kMiePhaseFunctionG,
		ozone_density, absorption_extinction, ground_albedo, max_sun_zenith_angle,
		kLengthUnitInMeters, use_luminance_ == PRECOMPUTED ? 15 : 3,
		use_combined_textures_, use_half_precision_));
	atmosphereScattering->Init();

	//Linking shader program
	atmosphereScatteringShader.SetShader("shader/AtmosphereVShader.glsl", "shader/AtmosphereFShader.glsl");
	glAttachShader(atmosphereScatteringShader.GetShader()->GetProgram(), atmosphereScattering->shader());
	glLinkProgram(atmosphereScatteringShader.GetShader()->GetProgram());
	GLint code;
	glGetProgramiv(atmosphereScatteringShader.GetShader()->GetProgram(), GL_LINK_STATUS, &code);

	if (!code) {
		cout << "Shader for atmospheric scattering set up failed!" << endl;
	}

	glDetachShader(atmosphereScatteringShader.GetShader()->GetProgram(), atmosphereScattering->shader());
	atmosphereScatteringShader.GetMesh()->CreatePlane();

	//Set 3 constant uniforms in fragment shader
	glUseProgram(atmosphereScatteringShader.GetShader()->GetProgram());
	atmosphereScattering->SetProgramUniforms(atmosphereScatteringShader.GetShader()->GetProgram(), 0, 1, 2, 3);
	double white_point_r = 1.0;
	double white_point_g = 1.0;
	double white_point_b = 1.0;
	if (do_white_balance_) {
		atmosphere::Model::ConvertSpectrumToLinearSrgb(wavelengths, solar_irradiance,
			&white_point_r, &white_point_g, &white_point_b);
		double white_point = (white_point_r + white_point_g + white_point_b) / 3.0;
		white_point_r /= white_point;
		white_point_g /= white_point;
		white_point_b /= white_point;
	}
	glUniform3f(glGetUniformLocation(atmosphereScatteringShader.GetShader()->GetProgram(), "white_point"),
		white_point_r, white_point_g, white_point_b);
	glUniform3f(glGetUniformLocation(atmosphereScatteringShader.GetShader()->GetProgram(), "earth_center"),
		0.0, 0.0, -kBottomRadius / kLengthUnitInMeters );
	glUniform2f(glGetUniformLocation(atmosphereScatteringShader.GetShader()->GetProgram(), "sun_size"),
		tan(kSunAngularRadius),
		cos(kSunAngularRadius));

	//Don't know why it is needed, probably due to precomputation
	glViewport(0, 0, width, height);

	glUniformMatrix4fv(glGetUniformLocation(atmosphereScatteringShader.GetShader()->GetProgram(), "projMatrix"), 1, GL_FALSE,
		(float*)&projMatrix);

	glUseProgram(0);
}

void Renderer::RenderAtmosphericScatteringModel()
{
	//Redundant parameter data similar to the CreateAtmosphericScatteringModel()
	enum Luminance {NONE, APPROXIMATE, PRECOMPUTED};
	bool	use_luminance_ = NONE;
	float	exposure_ = 10.0;

	Vector3 sunDir = camera->GetSunDirection();
	Vector3 cameraPos = camera->GetPosition();

	glUseProgram(atmosphereScatteringShader.GetShader()->GetProgram());
	//For some reasons, coordinate system in Bruneton's model is in order z,x,y
	//whereas OpenGL has x,y,z order.
	//PS: This adjustment also applies to the vertex shader when generating view ray.
	glUniform3f(glGetUniformLocation(atmosphereScatteringShader.GetShader()->GetProgram(), "camera"),
		cameraPos.z, cameraPos.x, cameraPos.y);
	glUniform1f(glGetUniformLocation(atmosphereScatteringShader.GetShader()->GetProgram(), "exposure"),
		use_luminance_ != NONE ? exposure_ * 1e-5 : exposure_);
	glUniformMatrix4fv(glGetUniformLocation(atmosphereScatteringShader.GetShader()->GetProgram(), "viewMatrix"),
		1, true, (float*)&camera->BuildViewMatrix());
	glUniform3f(glGetUniformLocation(atmosphereScatteringShader.GetShader()->GetProgram(), "sun_direction"),
		sunDir.z, sunDir.x, sunDir.y);

	atmosphereScatteringShader.Draw();
	glUseProgram(0);
}

void Renderer::CreateShadowMap()
{
	if (!shadowMappingShader.SetShader("shader/ShadowMapVShader.glsl", "shader/ShadowMapFShader.glsl")) {
		cout << "Shader set up failed!" << endl;
	}

	shadowFBO.reset(new FrameBuffer(SHADOWWIDTH, SHADOWHEIGHT, FBOCreationType::DEPTH));
}

void Renderer::RenderShadowMap()
{
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SHADOWWIDTH, SHADOWHEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO->GetFrameBuffer());
	glClear(GL_DEPTH_BUFFER_BIT);
	
	lightMatrix = Matrix4::BuildViewMatrix(light1->GetPosition(), Vector3(0, 0, 0));
	lightMatrix = projMatrix * lightMatrix * modelMatrix;

	glUseProgram(shadowMappingShader.GetShader()->GetProgram());
	glUniformMatrix4fv(glGetUniformLocation(shadowMappingShader.GetShader()->GetProgram(), "lightMatrix"), 1, GL_FALSE, (float*)&lightMatrix);

	object->Draw();

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
}

void Renderer::CreateSkybox()
{
	skybox = new RenderObject();
	if (!skybox->SetShader("shader/SkyBoxVShader.glsl", "shader/SkyBoxFShader.glsl")) {
		cout << "Shader set up failed!" << endl;
	}

	skybox->GetTexture()->CreateCubeMap("../assets/Skybox/bluecloud_rt.jpg",
		"../assets/Skybox/bluecloud_lf.jpg",
		"../assets/Skybox/bluecloud_up.jpg",
		"../assets/Skybox/bluecloud_dn.jpg",
		"../assets/Skybox/bluecloud_bk.jpg",
		"../assets/Skybox/bluecloud_ft.jpg");
	skybox->GetMesh()->CreateQuad();

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetTexture()->GetTexture());
}

void Renderer::CreateTrajectory()
{
	trajectory = new RenderObject();

	if (!trajectory->SetShader("shader/TriangleVShader.glsl", "shader/TriangleFShader.glsl")) {
		cout << "Shader set up failed!" << endl;
	}

	trajectory->SetMesh(new Trajectory());
}

void Renderer::ImGUIInit(Window& parent)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(parent.GetHandle());
	ImGui_ImplOpenGL3_Init();
}

void Renderer::RenderImGUI()
{	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//This demo showcases most of the features of Dear ImGUI
	//ImGui::ShowDemoWindow((bool*)true);
	ImGui::SliderFloat("Cloud global coverage", &cloudModel->globalCoverage, 0.0f, 1.0f);
	ImGui::SliderFloat("Cloud global density", &cloudModel->globalDensity, 0.0f, 1.0f);
	ImGui::SliderFloat("Cloud scale", &cloudModel->cloudScale, 0.1f, 10.f);
	ImGui::SliderFloat("Cloud offset", &cloudModel->cloudOffset, 0.f, 5.f);

	ImGui::SliderInt("Sample steps", &cloudModel->sampleSteps, 1, 128);
	ImGui::SliderInt("Light sample steps", &cloudModel->lightSampleSteps, 1, 10);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::UpdateControl(float msec)
{
	if (Window::GetKeyboard()->KeyHeld(KEYBOARD_CONTROL)) {
		Vector3 temp = light1->GetPosition();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
			temp.z += msec;
			light1->SetPosition(temp);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
			temp.x -= msec;
			light1->SetPosition(temp);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
			temp.z -= msec;
			light1->SetPosition(temp);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
			temp.x += msec;
			light1->SetPosition(temp);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
			temp.y += msec;
			light1->SetPosition(temp);
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
			temp.y -= msec;
			light1->SetPosition(temp);
		}
	}
}

void Renderer::ScreenShot(std::string filename)
{
	int row_size = ((width * 3 + 3) & ~3);
	int data_size = row_size * height;
	unsigned char* data = new unsigned char[data_size];
	glReadPixels(0,0,width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_flip_vertically_on_write(true);
	stbi_write_jpg(("../demo/" + filename + ".jpg").c_str(), width, height, 3, data, 100);
	delete[] data;
}