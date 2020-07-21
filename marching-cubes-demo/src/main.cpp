#include <Windows.h>
#include <Windowsx.h>

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

#include <vector>
#include <random>
#include <functional>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "MarchingCubesTables.h"

namespace Colors {
	XMGLOBALCONST DirectX::XMFLOAT4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMFLOAT4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMFLOAT4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMFLOAT4 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMFLOAT4 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMFLOAT4 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMFLOAT4 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMFLOAT4 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMFLOAT4 Grey = { 0.5f, 0.5f, 0.5f, 1.0f };
}

typedef struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT3 normal;
} Vertex;

// Window parameters
int screen_width = 1280;
int screen_height = 720;

// Direct3D device and context
ID3D11Device* d3d_device;
ID3D11DeviceContext* d3d_context;

// Vertex buffer, its buffer description and its subresource data
Vertex* vertex_buffer_data = nullptr;
int vertices_count = 0;
D3D11_BUFFER_DESC vertex_buffer_desc;
D3D11_SUBRESOURCE_DATA vertex_subresource_data;
ID3D11Buffer* vertex_buffer = nullptr;
UINT stride = sizeof(Vertex);
UINT offset = 0;

// Vertex indices buffer, its buffer description and its subresource data
UINT* vertex_indices_data = nullptr;
int indices_count = 0;
D3D11_BUFFER_DESC vertex_indices_desc;
D3D11_SUBRESOURCE_DATA vertex_indices_subresource_data;
ID3D11Buffer* vertex_index_buffer = nullptr;

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(0, 1);
auto random = std::bind(distribution, generator);

// Marching cube parameters
float threshold = 0.5f;
int resolution = 10;
bool interpolation = true;
void generate_marching_cubes_mesh() {
	// Traverse the space with the given resolution storing random values for each point
	std::vector<float> grid;
	grid.reserve(resolution * resolution * resolution);
	for (int i = 0; i <= 2; i += 2.0f/resolution) {
		for (int j = 0; j <= 2; j += 2.0f / resolution) {
			for (int k = 0; k <= 2; k += 2.0f / resolution) {
				grid.push_back(random());
			}
		}
	}
	// Traverse again with the marching cubes algorithm
	for (int i = 0; i < 2; i += 2.0f / resolution) {
		for (int j = 0; j < 2; j += 2.0f / resolution) {
			for (int k = 0; k < 2; k += 2.0f / resolution) {
				// Get configuration index
				int grid_index = resolution*resolution * i + resolution * j + k;
				int cube_index = 0;
				if (grid[grid_index] < threshold) { cube_index |= 1; }
				if (grid[grid_index + 1] < threshold) { cube_index |= 2; }
				if (grid[grid_index + resolution] < threshold) { cube_index |= 4; }
				if (grid[grid_index + resolution + 1] < threshold) { cube_index |= 8; }
				if (grid[grid_index + resolution * resolution] < threshold) { cube_index |= 16; }
				if (grid[grid_index + resolution * resolution + 1] < threshold) { cube_index |= 32; }
				if (grid[grid_index + resolution * resolution + resolution] < threshold) { cube_index |= 64; }
				if (grid[grid_index + resolution * resolution + resolution + 1] < threshold) { cube_index |= 128; }
				// Lookup the edge and triangle table with that index
				// Add vertices and indices of new triangles to buffer
			}
		}
	}
}

// Camera
DirectX::XMVECTOR camera_position = DirectX::XMVectorSet(0, 0, 5, 1);
DirectX::XMVECTOR camera_lookat_vector = DirectX::XMVectorSet(0, 0, -1, 1);
DirectX::XMVECTOR camera_right = DirectX::XMVectorSet(1, 0, 0, 1);
DirectX::XMVECTOR camera_up = DirectX::XMVectorSet(0, 1, 0, 1);
float near_plane = 0.1f;
float far_plane = 500.0f;
void rotate_camera_orbital(float angles_x, float angles_y) {
	// Create rotation quaternion for x axis
	float angle_x_rad = DirectX::XMConvertToRadians(angles_x / 2.0f);
	DirectX::XMFLOAT3 quaternion_x_imaginary(sinf(angle_x_rad) * camera_right.m128_f32[0], sinf(angle_x_rad) * camera_right.m128_f32[1], sinf(angle_x_rad) * camera_right.m128_f32[2]);
	float quaternion_x_real = cosf(angle_x_rad);
	DirectX::XMVECTOR quaternion_x = DirectX::XMVectorSet(quaternion_x_imaginary.x, quaternion_x_imaginary.y, quaternion_x_imaginary.z, quaternion_x_real);

	// Create rotation quaternion for y axis
	float angle_y_rad = DirectX::XMConvertToRadians(angles_y / 2.0f);
	DirectX::XMFLOAT3 quaternion_y_imaginary(0, sinf(angle_y_rad), 0);
	float quaternion_y_real = cosf(angle_y_rad);
	DirectX::XMVECTOR quaternion_y = DirectX::XMVectorSet(quaternion_y_imaginary.x, quaternion_y_imaginary.y, quaternion_y_imaginary.z, quaternion_y_real);

	// Combine quaternions
	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionNormalize(DirectX::XMQuaternionMultiply(quaternion_x, quaternion_y));

	//Apply result quaternion to camera position and right vector
	{
		DirectX::XMVECTOR intermediate_result = DirectX::XMQuaternionMultiply(quaternion, DirectX::XMVectorSet(camera_position.m128_f32[0], camera_position.m128_f32[1], camera_position.m128_f32[2], 0));
		intermediate_result = DirectX::XMQuaternionMultiply(intermediate_result, DirectX::XMQuaternionConjugate(quaternion));
		camera_position.m128_f32[0] = intermediate_result.m128_f32[0];
		camera_position.m128_f32[1] = intermediate_result.m128_f32[1];
		camera_position.m128_f32[2] = intermediate_result.m128_f32[2];
	}
	{
		DirectX::XMVECTOR intermediate_result = DirectX::XMQuaternionMultiply(quaternion_y, DirectX::XMVectorSet(camera_right.m128_f32[0], camera_right.m128_f32[1], camera_right.m128_f32[2], 0));
		intermediate_result = DirectX::XMQuaternionMultiply(intermediate_result, DirectX::XMQuaternionConjugate(quaternion_y));
		camera_right.m128_f32[0] = intermediate_result.m128_f32[0];
		camera_right.m128_f32[1] = intermediate_result.m128_f32[1];
		camera_right.m128_f32[2] = intermediate_result.m128_f32[2];
	}

	// Update camera lookat and up vectors
	camera_lookat_vector = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMVectorSet(0, 0, 0, 2), camera_position));
	camera_lookat_vector.m128_f32[3] = 1;
	camera_up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(camera_lookat_vector, DirectX::XMVectorNegate(camera_right)));
	camera_up.m128_f32[3] = 1;

	// Create perspective transform and bind it to the vertex shader
	DirectX::XMMATRIX persp_transf;
	persp_transf = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(camera_position, camera_lookat_vector, camera_up) * DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PI / 4.0f, float(screen_width) / float(screen_height), near_plane, far_plane));
	D3D11_BUFFER_DESC transform_desc;
	transform_desc.ByteWidth = sizeof(DirectX::XMMATRIX);
	transform_desc.Usage = D3D11_USAGE_DYNAMIC;
	transform_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transform_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	transform_desc.MiscFlags = 0;
	transform_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA transform_subres_data;
	transform_subres_data.pSysMem = &persp_transf;
	ID3D11Buffer* transform_buffer;
	d3d_device->CreateBuffer(&transform_desc, &transform_subres_data, &transform_buffer);
	d3d_context->VSSetConstantBuffers(0, 1, &transform_buffer);

	// Create buffer with camera position
	D3D11_BUFFER_DESC cam_pos_desc;
	cam_pos_desc.ByteWidth = sizeof(DirectX::XMVECTOR);
	cam_pos_desc.Usage = D3D11_USAGE_DYNAMIC;
	cam_pos_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cam_pos_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cam_pos_desc.MiscFlags = 0;
	cam_pos_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA cam_pos_subres_data;
	cam_pos_subres_data.pSysMem = &camera_position;
	ID3D11Buffer* cam_pos_buffer;
	d3d_device->CreateBuffer(&cam_pos_desc, &cam_pos_subres_data, &cam_pos_buffer);
	d3d_context->VSSetConstantBuffers(1, 1, &cam_pos_buffer);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool mouse_clicked = false;
int previous_pos_x = -1;
int previous_pos_y = -1;
LRESULT CALLBACK WindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	PAINTSTRUCT paintStruct;
	HDC hDC;
	switch (message) {
	case WM_PAINT:
		hDC = BeginPaint(hwnd, &paintStruct);
		EndPaint(hwnd, &paintStruct);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		mouse_clicked = true;
		previous_pos_x = GET_X_LPARAM(lParam);
		previous_pos_y = GET_Y_LPARAM(lParam);
		break;
	case WM_LBUTTONUP:
		mouse_clicked = false;
		previous_pos_x = -1;
		previous_pos_y = -1;
		break;
	case WM_MOUSEMOVE:
		if (mouse_clicked) {
			int pos_x = GET_X_LPARAM(lParam);
			int pos_y = GET_Y_LPARAM(lParam);

			rotate_camera_orbital((pos_y - previous_pos_y) / 3.5f, 0);
			rotate_camera_orbital(0, (pos_x - previous_pos_x) / 3.5f);

			previous_pos_x = pos_x;
			previous_pos_y = pos_y;
		}
		break;
	case WM_MOUSEWHEEL:
	{
		int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		camera_position.m128_f32[0] *= 1.0f - (delta / 500.0f);
		camera_position.m128_f32[1] *= 1.0f - (delta / 500.0f);
		camera_position.m128_f32[2] *= 1.0f - (delta / 500.0f);

		// Create perspective transform and bind it to the vertex shader
		DirectX::XMMATRIX persp_transf;
		persp_transf = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(camera_position, camera_lookat_vector, camera_up) * DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PI / 4.0f, float(screen_width) / float(screen_height), near_plane, far_plane));
		D3D11_BUFFER_DESC transform_desc;
		transform_desc.ByteWidth = sizeof(DirectX::XMMATRIX);
		transform_desc.Usage = D3D11_USAGE_DYNAMIC;
		transform_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		transform_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		transform_desc.MiscFlags = 0;
		transform_desc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA transform_subres_data;
		transform_subres_data.pSysMem = &persp_transf;
		ID3D11Buffer* transform_buffer;
		d3d_device->CreateBuffer(&transform_desc, &transform_subres_data, &transform_buffer);
		d3d_context->VSSetConstantBuffers(0, 1, &transform_buffer);
		break;
	}
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow) {
	// Initialize and show window
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WindowCallback;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = L"DX11BookWindowClass";
	if (!RegisterClassEx(&wndClass))
		return -1;
	RECT rc = { 0, 0, screen_width, screen_height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hwnd = CreateWindowA("DX11BookWindowClass", "Marching Cubes Demo",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.
		left,
		rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!hwnd)
		return -1;
	ShowWindow(hwnd, cmdShow);

	// Initialize Direct3D 11
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel;
	IDXGISwapChain* swap_chain;
	// Specify swap chain settings
	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	swap_chain_desc.BufferDesc.Width = screen_width;
	swap_chain_desc.BufferDesc.Height = screen_height;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 0;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.OutputWindow = hwnd;
	swap_chain_desc.Windowed = true;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.Flags = 0;
	// Create Direct3D device, context and swap chain
	D3D11CreateDeviceAndSwapChain(
		nullptr, // Default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr, // No software device
		createDeviceFlags,
		nullptr, 0, // Default feature level array
		D3D11_SDK_VERSION, // Direct3D 11 SDK version
		&swap_chain_desc,
		&swap_chain,
		&d3d_device,
		&featureLevel,
		&d3d_context
	);

	// Create depth and stencil buffers
	ID3D11Texture2D* depth_stencil_buffer;
	D3D11_TEXTURE2D_DESC depth_stencil_buffer_desc;
	depth_stencil_buffer_desc.Width = screen_width;
	depth_stencil_buffer_desc.Height = screen_height;
	depth_stencil_buffer_desc.MipLevels = 1;
	depth_stencil_buffer_desc.ArraySize = 1;
	depth_stencil_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_buffer_desc.SampleDesc.Count = 1;
	depth_stencil_buffer_desc.SampleDesc.Quality = 0;
	depth_stencil_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_buffer_desc.CPUAccessFlags = 0;
	depth_stencil_buffer_desc.MiscFlags = 0;
	d3d_device->CreateTexture2D(&depth_stencil_buffer_desc, nullptr, &depth_stencil_buffer);
	ID3D11DepthStencilView* depth_stencil_view;
	d3d_device->CreateDepthStencilView(depth_stencil_buffer, nullptr, &depth_stencil_view);

	// Create the render target view
	ID3D11RenderTargetView* render_target_view;
	ID3D11Texture2D* backbuffer;
	swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer));
	d3d_device->CreateRenderTargetView(backbuffer, 0, &render_target_view);
	backbuffer->Release();

	// Bind the render target and depth/stencil buffer to the output merger
	d3d_context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);

	// Create the viewport and bind it
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)screen_width;
	viewport.Height = (float)screen_height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	d3d_context->RSSetViewports(1, &viewport);

	// Set culling mode to clockwise because we use a right handed coordinate system
	D3D11_RASTERIZER_DESC rasterizer_desc;
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.FrontCounterClockwise = true;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.SlopeScaledDepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0;
	rasterizer_desc.DepthClipEnable = true;
	rasterizer_desc.ScissorEnable = false;
	rasterizer_desc.MultisampleEnable = false;
	rasterizer_desc.AntialiasedLineEnable = false;
	ID3D11RasterizerState* rasterizer_state;
	d3d_device->CreateRasterizerState(&rasterizer_desc, &rasterizer_state);
	d3d_context->RSSetState(rasterizer_state);

	// Create vertex format description
	D3D11_INPUT_ELEMENT_DESC vertex_desc_buffer[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	// Create and bind vertex input layout and read vertex shader
	ID3D11InputLayout* input_layout;
	ID3DBlob* vertex_shader_blob;
	D3DReadFileToBlob(L"VertexShader.cso", &vertex_shader_blob);
	d3d_device->CreateInputLayout(vertex_desc_buffer,
		sizeof(vertex_desc_buffer) / sizeof(D3D11_INPUT_ELEMENT_DESC),
		vertex_shader_blob->GetBufferPointer(),
		vertex_shader_blob->GetBufferSize(), &input_layout);
	d3d_context->IASetInputLayout(input_layout);

	// Create and set vertex shader
	ID3D11VertexShader* vertex_shader;
	d3d_device->CreateVertexShader(vertex_shader_blob->GetBufferPointer(),
		vertex_shader_blob->GetBufferSize(),
		nullptr,
		&vertex_shader);
	d3d_context->VSSetShader(vertex_shader, nullptr, 0);

	// Create and set pixel shader
	ID3DBlob* pixel_shader_blob;
	D3DReadFileToBlob(L"PixelShader.cso", &pixel_shader_blob);
	ID3D11PixelShader* pixel_shader;
	d3d_device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &pixel_shader);
	d3d_context->PSSetShader(pixel_shader, nullptr, 0);

	// Create perspective transform and bind ti to the vertex shader
	DirectX::XMMATRIX persp_transf;
	persp_transf = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(camera_position, camera_lookat_vector, camera_up) * DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PI / 4.0f,
		float(screen_width) / float(screen_height),
		near_plane,
		far_plane));
	D3D11_BUFFER_DESC transform_desc;
	transform_desc.ByteWidth = sizeof(DirectX::XMMATRIX);
	transform_desc.Usage = D3D11_USAGE_DYNAMIC;
	transform_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transform_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	transform_desc.MiscFlags = 0;
	transform_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA transform_subres_data;
	transform_subres_data.pSysMem = &persp_transf;
	ID3D11Buffer* transform_buffer;
	d3d_device->CreateBuffer(&transform_desc, &transform_subres_data, &transform_buffer);
	d3d_context->VSSetConstantBuffers(0, 1, &transform_buffer);

	// Create buffer with camera position
	D3D11_BUFFER_DESC cam_pos_desc;
	cam_pos_desc.ByteWidth = sizeof(DirectX::XMVECTOR);
	cam_pos_desc.Usage = D3D11_USAGE_DYNAMIC;
	cam_pos_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cam_pos_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cam_pos_desc.MiscFlags = 0;
	cam_pos_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA cam_pos_subres_data;
	cam_pos_subres_data.pSysMem = &camera_position;
	ID3D11Buffer* cam_pos_buffer;
	d3d_device->CreateBuffer(&cam_pos_desc, &cam_pos_subres_data, &cam_pos_buffer);
	d3d_context->VSSetConstantBuffers(1, 1, &cam_pos_buffer);

	// Initialize cube
	Vertex* cube_buffer_data = new Vertex[8];
	cube_buffer_data[0] = { {-1, 1, -1}, Colors::Grey, {-1, 1, -1} };
	cube_buffer_data[1] = { {1, 1, -1}, Colors::Grey, {1, 1, -1} };
	cube_buffer_data[2] = { {1, 1, 1}, Colors::Grey, {1, 1, 1} };
	cube_buffer_data[3] = { {-1, 1, 1}, Colors::Grey, {-1, 1, 1} };
	cube_buffer_data[4] = { {-1, -1, -1}, Colors::Grey, {-1, -1, -1} };
	cube_buffer_data[5] = { {1, -1, -1}, Colors::Grey, {1, -1, -1} };
	cube_buffer_data[6] = { {1, -1, 1}, Colors::Grey, {1, -1, 1} };
	cube_buffer_data[7] = { {-1, -1, 1}, Colors::Grey, {-1, -1, 1} };
	// Create cube vertex buffer
	D3D11_BUFFER_DESC cube_buffer_desc;
	cube_buffer_desc.ByteWidth = 8 * sizeof(Vertex);
	cube_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cube_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	cube_buffer_desc.CPUAccessFlags = 0;
	cube_buffer_desc.MiscFlags = 0;
	cube_buffer_desc.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA cube_subresource_data;
	cube_subresource_data.pSysMem = cube_buffer_data;
	// Create hardware cube vertex buffer
	ID3D11Buffer* cube_buffer = nullptr;
	d3d_device->CreateBuffer(&cube_buffer_desc, &cube_subresource_data, &cube_buffer);
	// Create cube indices data
	UINT* cube_indices_data = new UINT[24];
	cube_indices_data[0] = 0;
	cube_indices_data[1] = 1;
	cube_indices_data[2] = 1;
	cube_indices_data[3] = 2;
	cube_indices_data[4] = 2;
	cube_indices_data[5] = 3;
	cube_indices_data[6] = 3;
	cube_indices_data[7] = 0;
	cube_indices_data[8] = 4;
	cube_indices_data[9] = 5;
	cube_indices_data[10] = 5;
	cube_indices_data[11] = 6;
	cube_indices_data[12] = 6;
	cube_indices_data[13] = 7;
	cube_indices_data[14] = 7;
	cube_indices_data[15] = 4;
	cube_indices_data[16] = 0;
	cube_indices_data[17] = 4;
	cube_indices_data[18] = 1;
	cube_indices_data[19] = 5;
	cube_indices_data[20] = 2;
	cube_indices_data[21] = 6;
	cube_indices_data[22] = 3;
	cube_indices_data[23] = 7;
	// Create vertex indices buffer description
	D3D11_BUFFER_DESC cube_indices_desc;
	cube_indices_desc.ByteWidth = 24 * sizeof(UINT);
	cube_indices_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cube_indices_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	cube_indices_desc.CPUAccessFlags = 0;
	cube_indices_desc.MiscFlags = 0;
	cube_indices_desc.StructureByteStride = sizeof(UINT);
	D3D11_SUBRESOURCE_DATA cube_indices_subresource_data;
	cube_indices_subresource_data.pSysMem = cube_indices_data;
	// Create hardware vertex index buffer and bind it to the input assembler
	ID3D11Buffer* cube_index_buffer = nullptr;
	d3d_device->CreateBuffer(&cube_indices_desc, &cube_indices_subresource_data, &cube_index_buffer);
	// Create cube vertex shader
	ID3DBlob* cube_vertex_shader_blob;
	D3DReadFileToBlob(L"cube_vs.cso", &cube_vertex_shader_blob);
	ID3D11VertexShader* cube_vertex_shader;
	d3d_device->CreateVertexShader(cube_vertex_shader_blob->GetBufferPointer(),
		cube_vertex_shader_blob->GetBufferSize(),
		nullptr,
		&cube_vertex_shader);

	// Initialize initial camera position and orientation
	rotate_camera_orbital(30, 0);
	rotate_camera_orbital(0, -45);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(d3d_device, d3d_context);

	const FLOAT clear_color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	// Event loop
	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Update

		// Clear render target
		d3d_context->ClearRenderTargetView(render_target_view, clear_color);
		// Clear depth buffer
		d3d_context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Draw cube
		// Set vertex and index buffer of cube
		d3d_context->IASetVertexBuffers(0, 1, &cube_buffer, &stride, &offset);
		d3d_context->IASetIndexBuffer(cube_index_buffer, DXGI_FORMAT_R32_UINT, 0);
		// Set primitive topology type to line list
		d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		// Set cube vertex shader
		d3d_context->VSSetShader(cube_vertex_shader, nullptr, 0);
		//Draw mesh
		d3d_context->DrawIndexed(24, 0, 0);

		// Draw mesh if any has been read
		if (vertex_buffer_data && vertex_indices_data) {
			// Set vertex and index buffer
			d3d_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
			d3d_context->IASetIndexBuffer(vertex_index_buffer, DXGI_FORMAT_R32_UINT, 0);
			// Set primitive topology to triangle list
			d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			// Set vertex shader
			d3d_context->VSSetShader(vertex_shader, nullptr, 0);
			//Draw mesh
			d3d_context->DrawIndexed(indices_count, 0, 0);
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		// Render imgui widgets
		// TODO
		
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Trigger a back buffer swap in the swap chain
		swap_chain->Present(1, 0);
	}

	// Cleanup
	// ImGui cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Direct3D cleanup
	render_target_view->Release();
	swap_chain->Release();
	d3d_context->Release();
	d3d_device->Release();

	// Windows cleanup
	::DestroyWindow(hwnd);
	::UnregisterClass(wndClass.lpszClassName, wndClass.hInstance);

	return static_cast<int>(msg.wParam);
}