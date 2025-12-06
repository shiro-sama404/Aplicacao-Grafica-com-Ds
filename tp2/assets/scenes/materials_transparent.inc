include "colors.inc"

// Materiais opacos básicos
material "WhiteOpaque"
{
  ambient  White * 0.3
  diffuse  White * 0.7
  spot     White
  shine    60
  specular White * 0.5
  transparency Black
  ior 1.0
}

material "GrayOpaque"
{
  ambient  DarkGray
  diffuse  DarkGray
  spot     White
  shine    80
  specular White * 0.2
  transparency Black
  ior 1.0
}

material "RedOpaque"
{
  ambient  Red * 0.3
  diffuse  Red * 0.7
  spot     Red * 0.5
  shine    80
  specular White * 0.3
  transparency Black
  ior 1.0
}

material "BlueOpaque"
{
  ambient  Blue * 0.3
  diffuse  Blue * 0.7
  spot     Blue * 0.5
  shine    80
  specular White * 0.3
  transparency Black
  ior 1.0
}

material "GreenOpaque"
{
  ambient  Green * 0.3
  diffuse  Green * 0.7
  spot     Green * 0.5
  shine    80
  specular White * 0.3
  transparency Black
  ior 1.0
}

// Materiais transparentes - Vidros
material "ClearGlass"
{
  ambient  White * 0.1
  diffuse  White * 0.1
  spot     White
  shine    100
  specular White
  transparency White * 0.95
  ior 1.5
}

material "RedGlass"
{
  ambient  Red * 0.1
  diffuse  Red * 0.1
  spot     White
  shine    100
  specular White
  transparency rgb(0.95, 0.1, 0.1)
  ior 1.5
}

material "GreenGlass"
{
  ambient  Green * 0.1
  diffuse  Green * 0.1
  spot     White
  shine    100
  specular White
  transparency rgb(0.1, 0.95, 0.1)
  ior 1.5
}

material "BlueGlass"
{
  ambient  Blue * 0.1
  diffuse  Blue * 0.1
  spot     White
  shine    100
  specular White
  transparency rgb(0.1, 0.1, 0.95)
  ior 1.5
}

material "AmberGlass"
{
  ambient  Orange * 0.1
  diffuse  Orange * 0.1
  spot     White
  shine    100
  specular White
  transparency rgb(0.95, 0.7, 0.1)
  ior 1.5
}

// Água
material "Water"
{
  ambient  rgb(0.1, 0.3, 0.4) * 0.1
  diffuse  rgb(0.1, 0.3, 0.4) * 0.2
  spot     White * 0.8
  shine    90
  specular White * 0.9
  transparency rgb(0.85, 0.92, 0.95)
  ior 1.33
}

// Diamante
material "Diamond"
{
  ambient  White * 0.05
  diffuse  White * 0.05
  spot     White
  shine    120
  specular White
  transparency White * 0.98
  ior 2.42
}

// Materiais para teste de IOR
material "IOR_1_0"
{
  ambient  White * 0.1
  diffuse  White * 0.1
  spot     White
  shine    100
  specular White
  transparency White * 0.9
  ior 1.0
}

material "IOR_1_2"
{
  ambient  White * 0.1
  diffuse  White * 0.1
  spot     White
  shine    100
  specular White
  transparency White * 0.9
  ior 1.2
}

material "IOR_1_5"
{
  ambient  White * 0.1
  diffuse  White * 0.1
  spot     White
  shine    100
  specular White
  transparency White * 0.9
  ior 1.5
}

material "IOR_1_8"
{
  ambient  White * 0.1
  diffuse  White * 0.1
  spot     White
  shine    100
  specular White
  transparency White * 0.9
  ior 1.8
}

material "IOR_2_0"
{
  ambient  White * 0.1
  diffuse  White * 0.1
  spot     White
  shine    100
  specular White
  transparency White * 0.9
  ior 2.0
}

material "IOR_2_4"
{
  ambient  White * 0.1
  diffuse  White * 0.1
  spot     White
  shine    100
  specular White
  transparency White * 0.9
  ior 2.42
}

// Materiais para teste de transparência
material "Trans_0"
{
  ambient  Red * 0.3
  diffuse  Red * 0.7
  spot     Red * 0.5
  shine    80
  specular White * 0.3
  transparency Black
  ior 1.5
}

material "Trans_25"
{
  ambient  Red * 0.2
  diffuse  Red * 0.5
  spot     White * 0.5
  shine    80
  specular White * 0.5
  transparency White * 0.25
  ior 1.5
}

material "Trans_50"
{
  ambient  Red * 0.15
  diffuse  Red * 0.3
  spot     White * 0.7
  shine    90
  specular White * 0.7
  transparency White * 0.5
  ior 1.5
}

material "Trans_75"
{
  ambient  Red * 0.1
  diffuse  Red * 0.15
  spot     White * 0.9
  shine    95
  specular White * 0.9
  transparency White * 0.75
  ior 1.5
}

material "Trans_90"
{
  ambient  Red * 0.05
  diffuse  Red * 0.1
  spot     White
  shine    100
  specular White
  transparency White * 0.9
  ior 1.5
}

// Materiais para teste de shine
material "Shine_10"
{
  ambient  Blue * 0.3
  diffuse  Blue * 0.7
  spot     Blue * 0.5
  shine    10
  specular White * 0.3
  transparency Black
  ior 1.0
}

material "Shine_30"
{
  ambient  Blue * 0.3
  diffuse  Blue * 0.7
  spot     Blue * 0.5
  shine    30
  specular White * 0.3
  transparency Black
  ior 1.0
}

material "Shine_60"
{
  ambient  Blue * 0.3
  diffuse  Blue * 0.7
  spot     Blue * 0.5
  shine    60
  specular White * 0.3
  transparency Black
  ior 1.0
}

material "Shine_90"
{
  ambient  Blue * 0.3
  diffuse  Blue * 0.7
  spot     Blue * 0.5
  shine    90
  specular White * 0.3
  transparency Black
  ior 1.0
}

material "Shine_120"
{
  ambient  Blue * 0.3
  diffuse  Blue * 0.7
  spot     Blue * 0.5
  shine    120
  specular White * 0.3
  transparency Black
  ior 1.0
}