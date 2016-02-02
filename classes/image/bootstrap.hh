<?hh //strict
  namespace nova\image {
    enum ImageResizeType : int {
      Normal = 0;
      Fit = 1;
      FitAndCut = 2;
      VerticalMiddle = 10;
      HorizontalLeft = 11;
      VerticalFace = 20;
      VerticalTop = 21;
      VerticalTopPiece = 22;
      Auto = 100;
    }

    enum ImageSizeType : int {
      Horizontal = 0;
      HorizontalWide = 1;
      Vertical = 2;
      VerticalLong = 3;
      Square = 4;
    }

    type Rgba = shape("r" => int, "g" => int, "b" => int, "a" => int);
    type ColorDeviation = shape("r" => float, "g" => float, "b" => float, "a" => float);
    type SlicePosition = shape("y" => int, "diff" => float, "debug" => string);
    type Coordinate = shape("x" => int, "y" => int, "width" => int, "height" => int);
    type ResizeResult = shape("type" => ImageResizeType, "cropX" => int , "cropY" => int, "cropWidth" => int, "cropHeight" => int, "isTopPiece" => bool);
    type ImageSaveResult = shape("width" => int, "height" => int, "topPieceY" => int , "hash" => string, "items" => Map<string, ImageSaveResultItem>);
  }

  namespace nova\image\hash {
    enum ImageHashType : int {
      Average = 0;
      Difference = 1;
    }

  }
