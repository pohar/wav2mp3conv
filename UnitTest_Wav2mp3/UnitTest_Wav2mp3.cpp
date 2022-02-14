#include "pch.h"
#include "CppUnitTest.h"
#include "wav2mp3.h"
#include "helperf.h"
#include <stdexcept> 
#include <fstream>

using std::fstream;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTestWav2mp3
{
	TEST_CLASS(UnitTestWav2mp3)
	{
	public:

		TEST_METHOD(TestMethod1)
		{
			Assert::AreEqual(1, 1);
			Assert::AreNotEqual(1, 2);
		}
	};


	// TODO test HelperFunctions namespace

	TEST_CLASS(UnitTesWavReader)
	{
	public:

		TEST_METHOD(TestBadFilename)
		{
			WavReader wav(std::string("wav1/filenotfound.wav"));
			try
			{
				wav.ParseWavFile2();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			if (wav.IsValid())
			{
				Assert::Fail();
			}
			else
			{
				//ok
			}
		}
		TEST_METHOD(TestReadOkFile)
		{
			WavReader wav(std::string("wav1/Fanfare60.wav"));
			try
			{
				wav.ParseWavFile2();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			if (wav.IsValid())
			{
				int numsamples = wav.GetNumSamples();
				Assert::IsTrue(numsamples == 1323000);
			}
			else
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestReadBadFile)
		{
			WavReader wav(std::string("wav5/audiocheck.net_hdsweep_1Hz_88000Hz_-3dBFS_30s.wav"));
			try
			{
				wav.ParseWavFile2();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			if (wav.IsValid())
			{
				Assert::Fail();
			}
			else
			{
				// OK
			}
		}

		TEST_METHOD(TestReadBadFile32bitFloat)
		{
			WavReader wav(std::string("wav5/32_bit_float.wav"));
			try
			{
				wav.ParseWavFile2();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			if (wav.IsValid())
			{
				Assert::Fail();
			}
			else
			{
				// OK
			}
		}

		TEST_METHOD(TestGetters)
		{
			WavReader wav(std::string("wav5/file_example_WAV_2MG.wav"));
			try
			{
				wav.ParseWavFile2();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			if (wav.IsValid())
			{
				auto left_data = wav.GetLeftPCMData();
				Assert::IsNotNull(left_data);
				auto right_data = wav.GetLeftPCMData();
				Assert::IsNotNull(right_data);

				auto leftieee_data = wav.GetLeftIEEEData();
				Assert::IsNull(leftieee_data);
				auto rightieee_data = wav.GetLeftIEEEData();
				Assert::IsNull(rightieee_data);

				auto numchannels = wav.GetNumChannels();
				Assert::IsTrue(numchannels == 2);

				auto samplerate = wav.GetSampleRate();
				Assert::IsTrue(samplerate == 16000);

				auto byterate = wav.GetByteRate();
				Assert::IsTrue(byterate == 64000);

				auto numsamples = wav.GetNumSamples();
				Assert::IsTrue(numsamples == 536474);

				auto audioformat = wav.GetAudioFormat();
				Assert::IsTrue(audioformat == 1);
			}
			else
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestGettersIEEE)
		{
			WavReader wav(std::string("wav5/M1F1-float32-AFsp.wav"));
			try
			{
				wav.ParseWavFile2();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			if (wav.IsValid())
			{
				auto left_data = wav.GetLeftPCMData();
				Assert::IsNull(left_data);
				auto right_data = wav.GetLeftPCMData();
				Assert::IsNull(right_data);

				auto leftieee_data = wav.GetLeftIEEEData();
				Assert::IsNotNull(leftieee_data);
				auto rightieee_data = wav.GetLeftIEEEData();
				Assert::IsNotNull(rightieee_data);

				auto audioformat = wav.GetAudioFormat();
				Assert::IsTrue(audioformat == 3);
			}
			else
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestGettersBadIEEE)
		{
			WavReader wav(std::string("wav5/M1F1-float32WE-AFsp.wav"));
			try
			{
				wav.ParseWavFile2();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			if (wav.IsValid())
			{
				Assert::Fail();
			}
			else
			{
				// OK
			}
		}

	};

	TEST_CLASS(UnitTestConversionCoordinator)
	{
	public:

		TEST_METHOD(TestEmptyfilelist)
		{
			std::vector<std::string> WavFileList;
			//thread_params tp{ 0, WavFileList };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 0);
				Assert::IsTrue(cc.GetFilesSuccess() == 0);

			}
			catch (...)
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestFileNotFound)
		{
			std::vector<std::string> WavFileList = { "notfound.wav" };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 1);
				Assert::IsTrue(cc.GetFilesSuccess() == 0);
			}
			catch (...)
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestConvertSingleFile)
		{
			std::vector<std::string> WavFileList = { "wav1/Fanfare60.wav" };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 1);
				Assert::IsTrue(cc.GetFilesSuccess() == 1);
			}
			catch (...)
			{
				Assert::Fail();
			}
		}
		TEST_METHOD(TestConvertTwoFiles)
		{
			std::vector<std::string> WavFileList = { "wav2/lickpick.wav","wav2/StarWars60.wav" };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 2);
				Assert::IsTrue(cc.GetFilesSuccess() == 2);
			}
			catch (...)
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestConvertGoodAndBad)
		{
			std::vector<std::string> WavFileList = { "wav3/audiocheck.net_hdchirp_176k_-3dBFS_log.wav","wav3/audiocheck.net_hdchirp_88k_-3dBFS_log.wav" };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 2);
				Assert::IsTrue(cc.GetFilesSuccess() == 1);
			}
			catch (...)
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestConvertBigFile)
		{
			std::vector<std::string> WavFileList = { "wavok/Symphony No.6 (1st movement).wav" };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 1);
				Assert::IsTrue(cc.GetFilesSuccess() == 1);
			}
			catch (...)
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestConvertHugeFile)
		{
			std::vector<std::string> WavFileList = { "wavok/Rádiókabaré 2021.04.10..wav" };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 1);
				Assert::IsTrue(cc.GetFilesSuccess() == 1);
			}
			catch (...)
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestConvertBrokenFile)
		{
			std::vector<std::string> WavFileList = { "wav4/temple_of_love-sisters_of_mercy[BROKEN].wav" };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 1);
				Assert::IsTrue(cc.GetFilesSuccess() == 0);
			}
			catch (...)
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestConvertProblematicFiles)
		{
			std::vector<std::string> WavFileList = { "wavproblem/GLASS.wav", "wavproblem/Pmiscck.wav", "wavproblem/Ptjunk.wav", "wavproblem/Utopia Critical Stop.wav" };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 4);
				Assert::IsTrue(cc.GetFilesSuccess() == 1); // note: the file "Utopia Critical Stop.wav" passes the test, because the invalid/unknown chunk data is ignored
			}
			catch (...)
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestConvertMultichannel)
		{
			std::vector<std::string> WavFileList = { "wavmch/6_Channel_ID.wav", "wavmch/8_Channel_ID.wav" };
			try {
				ConversionCoordinator cc(WavFileList);
				cc.ConvertWavFiles();
				Assert::IsTrue(cc.GetFilesTried() == 2);
				Assert::IsTrue(cc.GetFilesSuccess() == 0);
			}
			catch (...)
			{
				Assert::Fail();
			}
		}

	};

	TEST_CLASS(UnitTestMP3Encoder)
	{
	public:

		TEST_METHOD(TestGoodFile)
		{
			WavReader wav(std::string("wav1/Fanfare60.wav"));
			try
			{
				wav.ParseWavFile2();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			if (wav.IsValid())
			{
				try
				{
					MP3Encoder mp3enc(std::string("wav1/Fanfare60.mp3"), wav.GetLeftPCMData(), wav.GetRightPCMData(), wav.GetNumSamples(), wav.GetNumChannels(), wav.GetSampleRate(), wav.GetByteRate());
					//				MP3Encoder(std::string&, const float*, const float*, int, int, int, int);
					//Assert::ExpectException();
					auto byteswritten = mp3enc.GetBytesWritten();
					Assert::IsTrue(byteswritten == 1201632);
				}
				catch (const std::exception& e)
				{
					std::cerr << e.what() << '\n';
					Assert::Fail();
				}
				catch (...)
				{
					Assert::Fail();
				}
			}
			else
			{
				Assert::Fail();
			}
		}

		TEST_METHOD(TestBadNumSamples)
		{
			WavReader wav(std::string("wav1/Fanfare60.wav"));
			try
			{
				wav.ParseWavFile2();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}

			if (wav.IsValid())
			{
				try
				{
					//throw std::runtime_error("runtime error!");
					//Assert::ExpectException<std::exception>([&]
						//{
							unsigned int numsamples = 10000000; // fictional number for too much samples required
							MP3Encoder mp3enc(std::string("wav1/Fanfare60.mp3"), wav.GetLeftPCMData(), wav.GetRightPCMData(), numsamples, wav.GetNumChannels(), wav.GetSampleRate(), wav.GetByteRate());
						//});
				}
				catch (std::exception e)
				{
					std::cerr << e.what() << '\n';
					Assert::Fail();
				}
				catch (...)
				{
					Assert::Fail();
					throw;
				}
			}
			else
			{
				Assert::Fail();
			}
		}

	};
}

/*
			//Assert::AreEqual(1, 1);
			//Assert::AreNotEqual(1, 2);
*/