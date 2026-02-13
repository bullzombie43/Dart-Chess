import 'package:flutter/material.dart';

const List<String> engineOptions = <String>['    ','MyEngineV1', 'MyEngineV2', 'Stockfish Level 1', 'Stockfish Level 2'];

typedef StringCallback = void Function(String value);

class EngineDropdownButton extends StatefulWidget {
  final StringCallback selectionCallback;

  const EngineDropdownButton({
    super.key,
    required this.selectionCallback
  });

  @override
  State<EngineDropdownButton> createState() => _EngineDropdownButtonState();
}

class _EngineDropdownButtonState extends State<EngineDropdownButton> {
  String dropdownValue = engineOptions.first;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        const Icon(Icons.arrow_drop_down, color: Colors.black),
        Expanded(
          child: DropdownButton<String>(
            value: dropdownValue,
            icon: const SizedBox.shrink(),
            isExpanded: true,
            elevation: 16,
            style: const TextStyle(color: Colors.black),
            underline: Container(height: 2, color: Colors.black),
            onChanged: (String? value) {
              setState(() {
                dropdownValue = value!;
                widget.selectionCallback(value);
              });
            },
            items: engineOptions.map<DropdownMenuItem<String>>((String value) {
              return DropdownMenuItem<String>(
                value: value,
                child: Text(
                  value.trim().isEmpty ? ' ' : value,
                  overflow: TextOverflow.ellipsis,
                ),
              );
            }).toList(),
          ),
        ),
      ],
    );
  }
}