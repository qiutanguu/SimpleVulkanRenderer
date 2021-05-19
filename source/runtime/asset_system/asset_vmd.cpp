#include "asset_vmd.h"

namespace flower { namespace asset{
	
	bool asset::ReadVMDFile(VMDFile* vmd,const char* filename)
	{
		std::ifstream infile;
		infile.open(filename,std::ios::binary);

		if(!infile.is_open())
		{
			LOG_IO_FATAL("�ļ�{0}��ʧ�ܣ�",filename);
			return false;
		}

		infile.seekg(0);
		LOG_IO_INFO("Reading Vmd file {0}.",filename);

		// 0. ��ȡheader
		infile.read((char*)&vmd->m_header.m_header[0],sizeof(char) * 30);
		infile.read((char*)&vmd->m_header.m_modelName[0],sizeof(char) * 20);

		if(std::string(vmd->m_header.m_header) != "Vocaloid Motion Data 0002" &&
			std::string(vmd->m_header.m_modelName) != "Vocaloid Motion Data"
			)
		{
			LOG_IO_ERROR("VMD Header error.");
			return false;
		}

		LOG_IO_INFO("VMD header : {0}.",std::string(vmd->m_header.m_header));
		LOG_IO_INFO("VMD model name : {0}.",std::string(vmd->m_header.m_modelName));

		// 1. ��ȡ����
		uint32_t motionCount = 0;
		infile.read((char*)&motionCount,sizeof(uint32_t));
		vmd->m_motions.resize(motionCount);
		for( auto& motion:vmd->m_motions )
		{
			infile.read((char*)&motion.m_boneName[0],sizeof(char) * 15);
			infile.read((char*)&motion.m_frame,sizeof(uint32_t));
			infile.read((char*)&motion.m_translate[0], sizeof(float) * 3);
			infile.read((char*)&motion.m_quaternion[0],sizeof(float) * 4);
			infile.read((char*)&motion.m_interpolation[0],sizeof(uint8_t) * 64);
		}

		if(!infile.eof())
		{
			// 2. ��ȡblendshape
			uint32_t blendShapeCount = 0;
			infile.read((char*)&blendShapeCount,sizeof(uint32_t));
			vmd->m_morphs.resize(blendShapeCount);
			for(auto& morph:vmd->m_morphs)
			{
				infile.read((char*)&morph.m_blendShapeName[0],sizeof(char)*15);
				infile.read((char*)&morph.m_frame,sizeof(uint32_t));
				infile.read((char*)&morph.m_weight,sizeof(float));
			}
		}
		

		if(!infile.eof())
		{
			// 3. ��ȡ���
			uint32_t cameraCount = 0;
			infile.read((char*)&cameraCount,sizeof(uint32_t));
			vmd->m_cameras.resize(cameraCount);
			for(auto& camera:vmd->m_cameras)
			{
				infile.read((char*)&camera.m_frame,sizeof(uint32_t));
				infile.read((char*)&camera.m_distance,sizeof(float));
				infile.read((char*)&camera.m_interest[0],sizeof(float) * 3);
				infile.read((char*)&camera.m_rotate[0],sizeof(float) * 3);
				infile.read((char*)&camera.m_interpolation[0],sizeof(uint8_t) * 24);
				infile.read((char*)&camera.m_viewAngle,sizeof(uint32_t));
				infile.read((char*)&camera.m_isPerspective,sizeof(uint8_t));
			}
		}

		if(!infile.eof())
		{
			// 4. ��ȡ�ƹ�
			uint32_t lightCount = 0;
			infile.read((char*)&lightCount,sizeof(uint32_t));
			vmd->m_lights.resize(lightCount);
			for(auto& light:vmd->m_lights)
			{
				infile.read((char*)&light.m_frame,sizeof(uint32_t));
				infile.read((char*)&light.m_color[0],sizeof(float) * 3);
				infile.read((char*)&light.m_position[0],sizeof(float) * 3);
			}
		}

		if(!infile.eof())
		{
			// 5. ��ȡ��Ӱ
			uint32_t shadowCount = 0;
			infile.read((char*)&shadowCount,sizeof(uint32_t));
			vmd->m_shadows.resize(shadowCount);
			for(auto& shadow:vmd->m_shadows)
			{
				infile.read((char*)&shadow.m_frame,sizeof(uint32_t));
				infile.read((char*)&shadow.m_shadowType,sizeof(uint8_t));
				infile.read((char*)&shadow.m_distance,sizeof(float));
			}
		}

		if(!infile.eof())
		{
			// 6. ��ȡik��Ϣ
			uint32_t ikCount = 0;
			infile.read((char*)&ikCount,sizeof(uint32_t));
			vmd->m_iks.resize(ikCount);
			for(auto& ik:vmd->m_iks)
			{
				infile.read((char*)&ik.m_frame,sizeof(uint32_t));
				infile.read((char*)&ik.m_show,sizeof(uint8_t));

				uint32_t ikInfoCount = 0;
				infile.read((char*)&ikInfoCount,sizeof(uint32_t));
				ik.m_ikInfos.resize(ikInfoCount);
				for(auto& ikInfo:ik.m_ikInfos)
				{
					infile.read((char*)&ikInfo.m_name,sizeof(char) * 20);
					infile.read((char*)&ikInfo.m_enable,sizeof(uint8_t));
				}
			}
		}

		if(!infile.eof())
		{
			LOG_IO_FATAL("ERROR");
		}

		infile.close();
		return true;
	}
	
}}