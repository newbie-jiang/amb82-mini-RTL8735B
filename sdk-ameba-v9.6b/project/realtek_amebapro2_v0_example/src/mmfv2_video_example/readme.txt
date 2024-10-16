Please follow the instructions for various options


	1. Uncomment the example you want to run in example_mmf2_video_surport()
	Note: If user want to use the examples, please check the defuault example "mmf2_video_example_v1_init" is comment
	
	2. cd project/realtek_amebapro2_v0_example/GCC-RELEASE

	3. mkdir build

	4. cd build

	5. cmake .. -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -DVIDEO_EXAMPLE=on

	6. cmake --build . --target flash
 
	Audio only examples:
	1.  mmf2_example_a_init:
		AmebaPro2's AAC sound stream over the network. The sound received by AmebaPro2 is encoded by AAC and then streamed through the network (rtsp).
	2.  mmf2_example_audioloop_init:
		The sound received by AmebaPro2 can be broadcast from the 3.5 audio channel of AmebaPro2, and the PCM transmission is directly used in the procedure.
	3.  mmf2_example_g711loop_init:
		The sound received by AmebaPro2 can be broadcast from the 3.5 audio channel of AmebaPro2. PCM is encoded by G711 and transmit, then decoded by G711 and playback.
	4.  mmf2_example_aacloop_init:
		The sound received by AmebaPro2 can be broadcast from the 3.5 audio channel of AmebaPro2. PCM is encoded by AAC and transmit, then decoded by AAD and playback.
	5.  mmf2_example_rtp_aad_init:
		Stream AAC sound over the network to AmebaPro2 for playback. Streaming audio is decoded by AAD and played through 3.5 audio jack.
	6.  mmf2_example_2way_audio_init:
		Stream AAC sound to AmebaPro2’s audio jack via the network and transmit the sound received by AmebaPro2 over the network simultaneously.
	7.  mmf2_example_pcmu_array_rtsp_init:
		Transmitting PCMU sound arrays within AmebaPro2 over the network.
	8.  mmf2_example_aac_array_rtsp_init:
		Transfer AAC sound arrays in AmebaPro2 over the network.
	9.  mmf2_example_opusloop_init:
		The sound received by AmebaPro2 can be broadcast from the 3.5 audio channel of AmebaPro2. PCM is encoded by OPUS and transmit, then decoded by OPUS and playback.
	10. mmf2_example_a_opus_init:
		AmebaPro2's OPUS sound stream over the network. The sound received by AmebaPro2 is encoded by OPUSC and then streamed through the network (rtsp).
	11. mmf2_example_rtp_opusd_init:
		Stream OPUSC sound over the network to AmebaPro2 for playback. Streaming audio is decoded by OPUSD and played through 3.5 audio jack.
	12. mmf2_example_2way_audio_opus_init:
		Stream OPUS sound to AmebaPro2’s audio jack via the network and transmit the sound received by AmebaPro2 over the network simultaneously.
	13. mmf2_example_pcm_array_audio_init:
		Play the array pcm data through AmebaPro2

	Video only examples:
	1.  mmf2_video_example_v1_init:
		Transfer AmebaPro2's H264/HEVC video stream over the network. Video default format: 720P 30FPS.
	2.  mmf2_video_example_v2_init:
		Transfer AmebaPro2's H264/HEVC video stream over the network. Video default format: 1080P 30FPS.
	3.  mmf2_video_example_v3_init:
		Transfer AmebaPro2's JPEG video stream over the network. Video default format: 1080P 30FPS.
	4.  mmf2_video_example_v1_shapshot_init:
		Transfer AmebaPro2's H264/HEVC video stream over the network and snapshot (JPEG) while streaming.
	5.  mmf2_video_example_simo_init:
		Transmitting two H264/HEVC video streams from AmebaPro2 over the network, the source of the video is the same video stream. Video default format: 1080P 30FPS.
	6.  mmf2_video_example_array_rtsp_init:
		Transfer H264/HEVC stream array in AmebaPro2 over the network. Video default format: 25FPS.
	7.  mmf2_video_example_v1_param_change_init:
		Transfer AmebaPro2's H264/HEVC video over the network and support dynamic adjustment of video parameters. The parameters of dynamic adjustment are Resolution, Rate Control Mode, Bit Rate in order.
	8.  mmf2_video_example_h264_array_mp4_init:
		AmebaPro2 will record H264/HEVC stream array to the SD card for 30 second. Video default format: 25FPS.
	9.  mmf2_video_example_md_rtsp_init:
		RTSP video stream over the network. MD detect motion and draw the motion region to RTSP channel.
	10. mmf2_video_example_v12_adjust_framerate_init:
		Transfer AmebaPro2's H264/HEVC video stream over the network. Video default format: 1080P 30FPS, and then adjust framerate when streaming on. Transfer AmebaPro2's H264/HEVC video stream over the network. Video default format: 720P 15FPS, and then adjust framerate when streaming on.
	11. mmf2_video_example_jpeg_external_init:
		Use video HW encode any data (NV12, NV16…) to jpeg. The results will be saved to SD card as test_0001.jpg, test_0002.jpg...
	12. mmf2_video_example_v1_rate_control_init:
		Example for auto rate control and manual rate control

	Video + Audio examples:
	1.  mmf2_video_example_av_init:
		Transfer AmebaPro2's H264/HEVC video and AAC sound stream over the network. Video default format: 1080P 30FPS.
	2.  mmf2_video_example_av2_init:
		Transmitting two H264/HEVC videos and AAC audio streams from AmebaPro2 over the network. The source of the videos is different ISP.
	3.  mmf2_video_example_av21_init:
		Transfer two copies of AmebaPro2's H264/HEVC video (1080P 30FPS) and AAC sound stream through the network, the video source is the same ISP channel.
	4.  mmf2_video_example_av_mp4_init:
		AmebaPro2 will record three videos (1080P 30FPS) to the SD card for 30 seconds each The default storage name is :
			AmebaPro2_recording_0.mp4
			AmebaPro2_recording_1.mp4
			AmebaPro2_recording_2.mp4
	5.  mmf2_video_example_av_rtsp_mp4_init:
		(1) Transfer AmebaPro2's H264/HEVC video and AAC sound stream over the network. Video default format: 1080P 30FPS.
		(2) AmebaPro2 will record three videos (1080P 30FPS+AAC) to the SD card for 30 seconds each. The default storage name is : AmebaPro2_recording_0.mp4 AmebaPro2_recording_1.mp4 AmebaPro2_recording_2.mp4 
		(3) Streaming AAC sounds to AmebaPro2 via the network.
		Note: (1) video source of (2) is from the same ISP channel.
	6.  mmf2_video_example_joint_test_init:
		(1) Transmitting two H264/HEVC video streams from AmebaPro2 over the network, the source of the video is the different video stream. Video default format: 1080P 30FPS (V1) and 720P 30FPS (V2).
		(2) Streaming two copies of AAC sounds to AmebaPro2 via the network.
	7.  mmf2_video_example_joint_test_rtsp_mp4_init:
		(1) Transfer AmebaPro2's H264/HEVC video and AAC sound stream over the network. Video default format: 1080P 30FPS.
		(2) AmebaPro2 will record three videos (720P 30FPS+AAC) to the SD card for 30 seconds each. The default storage name is : AmebaPro2_recording_0.mp4 AmebaPro2_recording_1.mp4 AmebaPro2_recording_2.mp4 
		(3) Streaming AAC sounds to AmebaPro2 via the network.
		(4) RTP send the audio stream from network to AmebaPro2 and the stream is decoded by AAD and played through 3.5 audio jack.
		Note: (1) video source of (2) is from different ISP channels.
	8.  mmf2_video_example_2way_audio_pcmu_doorbell_init:
		(1) Transmitting AmebaPro2's H264/HEVC stream and PCMU sound stream over the network. Video default format: 1080P 30FPS.
		(2) PCMU sound can be streamed to AmebaPro2 via the Internet and playback.
		(3) Play PCMU sound array in AmebaPro2 (default is the doorbell).
	9.  mmf2_video_example_2way_audio_pcmu_init:
		(1) Transmitting AmebaPro2's H264/HEVC stream and PCMU sound stream over the network. Video default format: 1080P 30FPS.
		(2) PCMU sound can be streamed to AmebaPro2 via the Internet and playback.
	10. mmf2_video_example_av_mp4_httpfs_init:
		AmebaPro2 will record a video every 30 seconds and save it to the SD card (1080P 30FPS+AAC). The default is to record 60 files, and repeat the recording after the end.
		The default storage name is: mp4_record_0.mp4~mp4_record_29.mp4 Also open Http File Server for client to do playback.
	11. mmf2_video_example_h264_pcmu_array_mp4_init:
		Save 1 video stream and 1 pcmu audio stream to mp4 file (the record file may not play on some player)
	12. mmf2_video_example_demuxer_rtsp_init:
		Demux a mp4 file (suggest to use a file created by AmebaPro2) and send the video and audio data through rtsp

	Video + NN examples:
	1.  mmf2_video_example_vipnn_rtsp_init:
		(1) RTSP video stream over the network.
		(2) NN do object detection and draw the bounding box to RTSP channel. (Please see NN chapter for more details)
	2.  mmf2_video_example_md_nn_rtsp_init:
		(1) RTSP video stream over the network.
		(2) MD module detect motion. If there is motion detected, it will trigger NN module to detect object and draw the bounding box to RTSP channel.
	3.  mmf2_video_example_vipnn_facedet_init:
		(1) RTSP video stream over the network.
		(2) NN do face detection then draw the bounding box and face landmark to RTSP channel. (Please see NN chapter for more details about how to load face detection NN model)
	4.  mmf2_video_example_face_rtsp_init:
		(1) RTSP video stream over the network.
		(2) NN do face detection and face recognition, and then draw the bounding box and face recognition result to RTSP channel. ((Please see NN chapter for more details about how to load face detection/recognition NN model)
	5.  mmf2_video_example_joint_test_all_nn_rtsp_init:
		(1) RTSP video stream over the network.
		(2) NN do object detection, face detection and face recognition, and then draw the bounding box and face recognition result to RTSP channel.
		NN do audio classification. (Please see NN chapter for more details about how to load face detection/recognition NN model)
	6.  mmf2_video_example_joint_test_vipnn_rtsp_mp4_init:
		(1) RTSP video stream over the network.
		(2) AmebaPro2 will record three videos (720P 30FPS+AAC) to the SD card for 30 seconds each. The default storage name is : AmebaPro2_recording_0.mp4 AmebaPro2_recording_1.mp4 AmebaPro2_recording_2.mp4 
		(3) Streaming AAC sounds to AmebaPro2 via the network.
		(4) RTP send the audio stream from network to AmebaPro2 and the stream is decoded by AAD and played through 3.5 audio jack.
		(5) NN do object detection, face detection and face recognition, and then draw the bounding box and face recognition result to RTSP channel.
		NN do audio classification. (Please see NN chapter for more details about how to load face detection/recognition NN model)
	7.  mmf2_video_example_vipnn_facedet_sync_init:
		(1) RTSP video stream over the network.
		(2) NN do face detection then draw the bounding box and face landmark to RTSP channel. (Please see NN chapter for more details about how to load face detection NN model)
	8.  mmf2_video_example_vipnn_facedet_sync_snapshot_init:
		NN do face detection then draw the bounding box and face landmark to JPEG. The results will be saved to SD as test_0001.jpg, test_0002.jpg...
	9.  mmf2_video_example_fd_lm_mfn_sim_rtsp_init:
		(1) RTSP video stream over the network.
		(2) 3 models cascading: face detection + face landmark detection + face recognition.

	Audio + NN examples:
	1.  mmf2_video_example_audio_vipnn_init:
		The sound received by AmebaPro2 can be transmitted to NN engine to do sound classification.
		Please see NN chapter for more details

	Video + Audio +FCS:
	1.  mmf2_video_example_joint_test_rtsp_mp4_init_fcs:
		Please see FCS chapter for more details