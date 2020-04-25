#include <stdio.h>
#include <stdint.h>
#include <d3dcompiler.h>

#pragma comment(lib, "D3DCompiler.lib")

struct CompileJob
{
	LPCWSTR m_filename;
	LPCWSTR m_outputFilename;
	LPCSTR m_shaderVarName;
	const D3D_SHADER_MACRO *m_defs;
	LPCSTR m_entryPoint;
	LPCSTR m_target;
};

const D3D_SHADER_MACRO drawQuadDefs[] =
{
	{ nullptr }
};

const CompileJob kCompileJobs[] =
{
	{
		L"ShaderSrc\\DrawQuadV.hlsl",
		L"GpDisplayDriver_D3D11\\CompiledShaders\\DrawQuadV_D3D11.cpp",
		"g_drawQuadV_D3D11",
		drawQuadDefs,
		"VSMain",
		"vs_4_0"
	},
	{
		L"ShaderSrc\\DrawQuadRGBP.hlsl",
		L"GpDisplayDriver_D3D11\\CompiledShaders\\DrawQuadRGBP_D3D11.cpp",
		"g_drawQuadRGBP_D3D11",
		drawQuadDefs,
		"PSMain",
		"ps_4_0"
	},
	{
		L"ShaderSrc\\DrawQuadPaletteP.hlsl",
		L"GpDisplayDriver_D3D11\\CompiledShaders\\DrawQuadPaletteP_D3D11.cpp",
		"g_drawQuadPaletteP_D3D11",
		drawQuadDefs,
		"PSMain",
		"ps_4_0"
	},
	{
		L"ShaderSrc\\DrawQuad15BitP.hlsl",
		L"GpDisplayDriver_D3D11\\CompiledShaders\\DrawQuad15BitP_D3D11.cpp",
		"g_drawQuad15BitP_D3D11",
		drawQuadDefs,
		"PSMain",
		"ps_4_0"
	},
	{
		L"ShaderSrc\\ScaleQuadP.hlsl",
		L"GpDisplayDriver_D3D11\\CompiledShaders\\ScaleQuadP_D3D11.cpp",
		"g_scaleQuadP_D3D11",
		drawQuadDefs,
		"PSMain",
		"ps_4_0"
	}
};

int main(int argc, const char **argv)
{
	const size_t numJobs = sizeof(kCompileJobs) / sizeof(kCompileJobs[0]);

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
	for (size_t i = 0; i < numJobs; i++)
	{
		const CompileJob &job = kCompileJobs[i];

		ID3DBlob *code = nullptr;
		ID3DBlob *errorMsgs = nullptr;

		HRESULT result = D3DCompileFromFile(job.m_filename, job.m_defs, D3D_COMPILE_STANDARD_FILE_INCLUDE, job.m_entryPoint, job.m_target, flags, 0, &code, &errorMsgs);

		if (result != S_OK)
		{
			fprintf(stderr, "Compile failure error: %x", static_cast<int>(result));
		}

		if (errorMsgs)
		{
			fputs(static_cast<const char*>(errorMsgs->GetBufferPointer()), stderr);
			errorMsgs->Release();
		}

		if (code)
		{
			FILE *f = nullptr;
			errno_t errorCode = _wfopen_s(&f, job.m_outputFilename, L"wb");

			if (!errorCode)
			{
				fprintf(f, "static unsigned char gs_shaderData[] = {");

				const uint8_t *bytes = static_cast<const uint8_t *>(code->GetBufferPointer());
				const size_t size = code->GetBufferSize();

				for (size_t i = 0; i < size; i++)
				{
					if (i % 15 == 0)
						fputs("\n\t", f);
					fprintf(f, "%3i, ", static_cast<int>(bytes[i]));
				}

				fprintf(f, "\n};\n\n");
				fprintf(f, "namespace GpBinarizedShaders\n{\n");
				fprintf(f, "\tconst unsigned char *%s[2] = { gs_shaderData, gs_shaderData + sizeof(gs_shaderData) }; \n", job.m_shaderVarName);
				fprintf(f, "};\n");
				fclose(f);
			}

			code->Release();
		}
	}

	return 0;
}
