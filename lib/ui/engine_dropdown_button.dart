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
    final theme = Theme.of(context);
    final colorScheme = theme.colorScheme;
    final textStyle = theme.textTheme.bodyLarge?.copyWith(
      color: colorScheme.onSurface,
    );
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(
          Icons.arrow_drop_down,
          color: colorScheme.onSurface,
        ),
        Expanded(
          child: DropdownButton<String>(
            value: dropdownValue,
            icon: const SizedBox.shrink(),
            isExpanded: true,
            elevation: 8,
            dropdownColor: colorScheme.surfaceContainerHighest,
            style: textStyle,
            underline: Container(
              height: 1,
              color: colorScheme.outlineVariant,
            ),
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
                  style: textStyle,
                ),
              );
            }).toList(),
          ),
        ),
      ],
    );
  }
}