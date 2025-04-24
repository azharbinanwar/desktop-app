import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:image_picker/image_picker.dart';
import 'dart:io';

import 'package:my_app/user_data_page.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Kiosk Project - Experimental',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const ImagePickerPage(),
      debugShowCheckedModeBanner: false,
    );
  }
}

class ImagePickerPage extends StatefulWidget {
  const ImagePickerPage({super.key});

  @override
  State<ImagePickerPage> createState() => _ImagePickerPageState();
}

class _ImagePickerPageState extends State<ImagePickerPage> {
  File? _image;
  final ImagePicker _picker = ImagePicker();

  // Add method channel
  static const platform = MethodChannel('com.example.my_app/system_info');
  String _systemInfo = 'Unknown';

  Future<void> _getImageFromGallery() async {
    final XFile? pickedFile = await _picker.pickImage(source: ImageSource.gallery);

    if (pickedFile != null) {
      setState(() {
        _image = File(pickedFile.path);
      });
    }
  }

  // Add method to call native code
  Future<void> _getSystemInfo() async {
    String systemInfo;
    try {
      final result = await platform.invokeMethod('getProcessorInfo');
      systemInfo = 'Processor: $result';
    } on PlatformException catch (e) {
      systemInfo = 'Failed to get system info: ${e.message}';
    }

    setState(() {
      _systemInfo = systemInfo;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Kiosk Project - Experimental'),
        actions: [
          IconButton(
            onPressed: () => Navigator.of(context).push(MaterialPageRoute(builder: (context) => const FormPage())),
            icon: const Icon(Icons.image),
          ),
        ],
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            _image == null ? const Text('No image selected') : Image.file(_image!, height: 300, fit: BoxFit.cover),
            const SizedBox(height: 20),
            ElevatedButton(onPressed: _getImageFromGallery, child: const Text('Pick Image from Gallery')),
            const SizedBox(height: 30),
            Text(_systemInfo),
            ElevatedButton(onPressed: _getSystemInfo, child: const Text('Get System Info')),
          ],
        ),
      ),
    );
  }
}
