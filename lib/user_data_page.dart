// main.dart
import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class FormPage extends StatefulWidget {
  const FormPage({Key? key}) : super(key: key);

  @override
  State<FormPage> createState() => _FormPageState();
}

class _FormPageState extends State<FormPage> {
  final _formKey = GlobalKey<FormState>();
  final _nameController = TextEditingController();
  final _fullNameController = TextEditingController();
  final _locationController = TextEditingController();
  DateTime? _selectedDate;

  // Method channel for C# communication
  static const platform = MethodChannel('com.example.my_app/form_handler');

  List<Map<String, dynamic>> _records = [];
  bool _isLoading = false;
  String _errorMessage = '';

  Future<void> _saveFormData() async {
    if (_formKey.currentState!.validate()) {
      setState(() {
        _isLoading = true;
        _errorMessage = '';
      });

      try {
        final result = await platform.invokeMethod('saveFormData', {
          'name': _nameController.text,
          'fullName': _fullNameController.text,
          'location': _locationController.text,
          'dateOfBirth': _selectedDate?.toIso8601String() ?? '',
        });
        debugPrint('_FormPageState._saveFormData: result $result');

        // Parse result as a Map
        Map<String, dynamic> response = jsonDecode(result);

        if (response['Success'] == true) {
          ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(response['Message'] ?? 'Data saved successfully')));
          _resetForm();
          _loadRecords();
        } else {
          setState(() {
            _errorMessage = response['error'] ?? 'Unknown error';
          });
          debugPrint('_FormPageState._saveFormData: Failed to save data: ${response['error']}');
          ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('Failed to save data: $_errorMessage')));
        }
      } catch (e, s) {
        setState(() {
          _errorMessage = e.toString();
        });
        debugPrint('_FormPageState._saveFormData: d $e \n$s');
        ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('Error: $e')));
      } finally {
        setState(() {
          _isLoading = false;
        });
      }
    }
  }

  Future<void> _loadRecords() async {
    setState(() {
      _isLoading = true;
      _errorMessage = '';
    });

    try {
      final result = await platform.invokeMethod('getFormData');

      if (result == null || result.toString().isEmpty) {
        setState(() {
          _errorMessage = "Received empty response from native code";
          _records = [];
        });
        return;
      }

      debugPrint("Received data: $result");

      try {
        final  response = jsonDecode(result.toString());

        if (response['Success'] == true) {
          setState(() {
            _records = List<Map<String, dynamic>>.from(response['Data'] ?? []);
          });
        } else {
          setState(() {
            _errorMessage = response['error'] ?? 'Unknown error';
            _records = [];
          });
        }
      } catch (e , s) {
        debugPrint('_FormPageState._loadRecords: Failed to parse JSON: $e\n$s');
        setState(() {
          _errorMessage = "Failed to parse JSON: $e";
          _records = [];
        });
      }
    } catch (e, s) {
      debugPrint('_FormPageState._loadRecords: $e\n$s');
      setState(() {
        _errorMessage = e.toString();
        _records = [];
      });
    } finally {
      setState(() {
        _isLoading = false;
      });
    }
  }

  void _resetForm() {
    _nameController.clear();
    _fullNameController.clear();
    _locationController.clear();
    setState(() {
      _selectedDate = null;
    });
  }

  Future<void> _selectDate(BuildContext context) async {
    final DateTime? picked = await showDatePicker(
      context: context,
      initialDate: _selectedDate ?? DateTime.now(),
      firstDate: DateTime(1900),
      lastDate: DateTime.now(),
    );
    if (picked != null && picked != _selectedDate) {
      setState(() {
        _selectedDate = picked;
      });
    }
  }

  @override
  void initState() {
    super.initState();
    _loadRecords();

    _nameController.text = 'Joseph Kamal';
    _fullNameController.text = 'Joseph Kamal';
    _locationController.text = 'Lahore';
    _selectedDate = DateTime(1990, 1, 1);
  }

  @override
  void dispose() {
    _nameController.dispose();
    _fullNameController.dispose();
    _locationController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Form Data Saver')),
      body:
          _isLoading
              ? const Center(child: CircularProgressIndicator())
              : Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    if (_errorMessage.isNotEmpty)
                      Container(
                        padding: const EdgeInsets.all(8),
                        margin: const EdgeInsets.only(bottom: 16),
                        color: Colors.red.shade100,
                        width: double.infinity,
                        child: Text('Error: $_errorMessage', style: TextStyle(color: Colors.red.shade800)),
                      ),
                    Form(
                      key: _formKey,
                      child: Column(
                        children: [
                          TextFormField(
                            controller: _nameController,
                            decoration: const InputDecoration(labelText: 'Name', border: OutlineInputBorder()),
                            validator: (value) {
                              if (value == null || value.isEmpty) {
                                return 'Please enter a name';
                              }
                              return null;
                            },
                          ),
                          const SizedBox(height: 16),
                          TextFormField(
                            controller: _fullNameController,
                            decoration: const InputDecoration(labelText: 'Full Name', border: OutlineInputBorder()),
                            validator: (value) {
                              if (value == null || value.isEmpty) {
                                return 'Please enter full name';
                              }
                              return null;
                            },
                          ),
                          const SizedBox(height: 16),
                          TextFormField(
                            controller: _locationController,
                            decoration: const InputDecoration(labelText: 'Location', border: OutlineInputBorder()),
                            validator: (value) {
                              if (value == null || value.isEmpty) {
                                return 'Please enter a location';
                              }
                              return null;
                            },
                          ),
                          const SizedBox(height: 16),
                          InkWell(
                            onTap: () => _selectDate(context),
                            child: InputDecorator(
                              decoration: const InputDecoration(labelText: 'Date of Birth', border: OutlineInputBorder()),
                              child: Row(
                                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                                children: [Text(_selectedDate == null ? 'Select Date' : _selectedDate!.toString()), const Icon(Icons.calendar_today)],
                              ),
                            ),
                          ),
                          const SizedBox(height: 24),
                          ElevatedButton(
                            onPressed: _saveFormData,
                            style: ElevatedButton.styleFrom(minimumSize: const Size.fromHeight(50)),
                            child: const Text('Save Data'),
                          ),
                        ],
                      ),
                    ),
                    const SizedBox(height: 24),
                    const Text('Saved Records', style: TextStyle(fontSize: 18, fontWeight: FontWeight.bold)),
                    const SizedBox(height: 8),
                    Expanded(
                      child:
                          _records.isEmpty
                              ? const Center(child: Text('No records found'))
                              : ListView.builder(
                                itemCount: _records.length,
                                itemBuilder: (context, index) {
                                  final record = _records[index];
                                  final String dob =
                                      record['dateOfBirth'] != null && record['dateOfBirth'].isNotEmpty ? record['dateOfBirth'] : 'Not specified';

                                  return Card(
                                    margin: const EdgeInsets.symmetric(vertical: 8),
                                    child: ListTile(
                                      title: Text('${record['name']} (${record['fullName']})'),
                                      subtitle: Column(
                                        crossAxisAlignment: CrossAxisAlignment.start,
                                        children: [Text('Location: ${record['location']}'), Text('DOB: $dob')],
                                      ),
                                    ),
                                  );
                                },
                              ),
                    ),
                  ],
                ),
              ),
    );
  }
}
