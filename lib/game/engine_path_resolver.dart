import 'dart:io';

/// Resolves engine names to file paths
/// 
/// Handles mapping between UI-friendly engine names and actual executable paths
class EnginePathResolver {
  final String projectRoot;
  
  EnginePathResolver({String? projectRoot}) 
      : projectRoot = projectRoot ?? '/Users/justin/VSCODE PROJECTS/chess_ui';
  
  /// Resolve engine name to file path
  /// 
  /// Supports:
  /// - Custom engine names (e.g., "MyEngineV1") -> resolves to test_versions directory
  /// - Stockfish variants (e.g., "Stockfish Level 1") -> resolves to system Stockfish
  /// - Direct paths (if name contains "/" or starts with absolute path)
  String? resolveEnginePath(String engineName) {
    // If it's already a path, return as-is
    if (engineName.contains('/') || engineName.startsWith('~/')) {
      return engineName;
    }
    
    // Handle Stockfish variants
    if (engineName.toLowerCase().contains('stockfish')) {
      // For now, always use system Stockfish
      // Skill level is set via UCI options, not path
      return '/opt/homebrew/bin/stockfish';
    }
    
    // Handle custom engines from test_versions directory
    final testVersionsPath = '$projectRoot/native/test_versions/$engineName';
    if (File(testVersionsPath).existsSync()) {
      return testVersionsPath;
    }
    
    // Try build directory (for current engine)
    final buildPath = '$projectRoot/native/build/chess_engine_uci';
    if (File(buildPath).existsSync()) {
      return buildPath;
    }
    
    // Not found
    return null;
  }
  
  /// Get UCI options for an engine based on its name
  /// 
  /// Returns a map of UCI option names to values
  Map<String, String> getEngineOptions(String engineName) {
    final options = <String, String>{};
    
    // Handle Stockfish skill level
    if (engineName.toLowerCase().contains('stockfish')) {
      final skillMatch = RegExp(r'level\s*(\d+)', caseSensitive: false).firstMatch(engineName);
      if (skillMatch != null) {
        final skillLevel = skillMatch.group(1);
        options['Skill Level'] = skillLevel ?? '1';
      } else {
        // Default skill level
        options['Skill Level'] = '1';
      }
    }
    
    return options;
  }
  
  /// Check if an engine path exists and is executable
  bool isEngineValid(String? enginePath) {
    if (enginePath == null) return false;
    
    final file = File(enginePath);
    if (!file.existsSync()) return false;
    
    // On Unix systems, check if executable
    if (Platform.isLinux || Platform.isMacOS) {
      // Check if file has execute permission
      final stat = file.statSync();
      return stat.mode & 0x111 != 0; // Check execute bits
    }
    
    // On Windows, just check if file exists
    return true;
  }
}
