///////////////////////////////////////////////////////////////////////////////////////////////
// sprite-utils: Tool used to work with sprites
// Copyright (C) 2024 the original author or authors.
//
// This program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see 
// <https://www.gnu.org/licenses/> or write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
///////////////////////////////////////////////////////////////////////////////////////////////
package org.nanoboot.spriteutils.commands;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Stroke;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.BiConsumer;
import java.util.function.BiFunction;
import java.util.function.Supplier;
import lombok.AllArgsConstructor;
import net.sf.image4j.codec.bmp.BMPDecoder;
import net.sf.image4j.codec.bmp.BMPEncoder;
import net.sf.image4j.codec.bmp.BMPImage;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.nanoboot.spriteutils.core.Command;
import org.nanoboot.spriteutils.core.SpriteSheet;
import org.nanoboot.spriteutils.core.SpriteSheetRow;
import org.nanoboot.spriteutils.core.SpriteUtilsArgs;
import org.nanoboot.spriteutils.core.SpriteUtilsException;
import org.nanoboot.spriteutils.core.SpriteUtilsOptions;
import org.nanoboot.spriteutils.core.Utils;

/**
 *
 * @author r
 */
public class DrawCommand implements Command {

    private static final Logger LOG = LogManager.getLogger(DrawCommand.class);
    public static final String NAME = "draw";

    public DrawCommand() {

    }

    @Override
    public String getName() {
        return NAME;
    }

    private static final BiFunction<Integer, Integer, Integer> random = (min, max) -> {
        int diff = max - min;
        return min + (int) (Math.random() * diff);
    };

    private static final Map<Character, boolean[]> digitData = new HashMap<>();
    
    private static final Map<Character, boolean[]> digitDataDoubleSized = new HashMap<>();

    static {
        digitData.put('0', new boolean[]{
            true , true , true ,
            true , false, true ,
            true , false, true ,
            true , false, true ,
            true , true , true });
        digitData.put('1', new boolean[]{
            false, false, true ,
            false, true , true ,
            true , false, true ,
            false, false, true ,
            false, false, true });
        digitData.put('2', new boolean[]{
            true , true , true ,
            false, false, true ,
            false, true , false,
            true , false, false,
            true , true , true });
        digitData.put('3', new boolean[]{
            true , true , true ,
            false, false, true ,
            true , true , true ,
            false, false, true ,
            true , true , true });
        digitData.put('4', new boolean[]{
            false, false, true ,
            false, true , false,
            true , true , true ,
            false, false, true ,
            false, false, true });
        digitData.put('5', new boolean[]{
            true , true , true ,
            true , false, false,
            true , true , true ,
            false, false, true ,
            true , true , true });
        digitData.put('6', new boolean[]{
            true , true , true ,
            true , false, false,
            true , true , true ,
            true , false, true ,
            true , true , true });
        digitData.put('7', new boolean[]{
            true , true , true ,
            false, false, true ,
            false, false, true ,
            false, false, true ,
            false, false, true });
        digitData.put('8', new boolean[]{
            true , true , true ,
            true , false, true ,
            true , true , true ,
            true , false, true ,
            true , true , true });
        digitData.put('9', new boolean[]{
            true , true , true ,
            true , false, true ,
            true , true , true ,
            false, false, true ,
            true , true , true });
    }

    static {
        digitDataDoubleSized.put('0', new boolean[]{
            false, true , true , true , true , false,
            true , true , true , true , true , true ,
            true , true , false, false, true , true ,
            true , true , false, false, true , true ,
            true , true , false, false, true , true ,
            true , true , false, false, true , true ,
            true , true , false, false, true , true ,
            true , true , false, false, true , true ,
            true , true , false, false, true , true ,
            false, true , true , true , true , false,});
        digitDataDoubleSized.put('1', new boolean[]{
            false, false, false, false, true , true ,
            false, false, false, true , true , true ,
            false, false, true , true , true , true ,
            false, true , true , true , true , true ,
            true , true , true , false, true , true ,
            true , true , false, false, true , true ,
            false, false, false, false, true , true ,
            false, false, false, false, true , true ,
            false, false, false, false, true , true ,
            false, false, false, false, true , true ,});
        digitDataDoubleSized.put('2', new boolean[]{
            false, true , true , true , true ,false,
            true , true , true , true , true ,true ,
            false, false, false, false, true, true ,
            false, false, false, true , true, true ,
            false, false, true , true , true, false,
            false, false, true , true , false, false,
            false, true , true , false, false, false,
            false, true , true , false, false, false,
            true , true , true , true , true , true ,
            true , true , true , true , true , true ,});
        digitDataDoubleSized.put('3', convertStringToBooleanArray(
"""
 ████ |
██████|
    ██|
    ██|
█████ |
█████ |
    ██|
    ██|
██████|
 ████ |
"""));
        digitDataDoubleSized.put('4', convertStringToBooleanArray(
"""
    ██|
   ███|
  ███ |
 ███  |
███   |
██████|
██████|
    ██|
    ██|
    ██|
"""));
        digitDataDoubleSized.put('5', convertStringToBooleanArray(
"""
██████|
██████|
██    |
██    |
█████ |
██████|
    ██|
    ██|
██████|
█████ |
"""));
        digitDataDoubleSized.put('6', convertStringToBooleanArray(
"""
 █████|
██████|
██    |
██    |
█████ |
██████|
██  ██|
██  ██|
██████|
 ████ |
"""));
        digitDataDoubleSized.put('7', convertStringToBooleanArray(
"""
██████|
██████|
    ██|
    ██|
    ██|
    ██|
    ██|
    ██|
    ██|
    ██|
"""));
        digitDataDoubleSized.put('8', convertStringToBooleanArray(
"""
 ████ |
██████|
██  ██|
██  ██|
 ████ |
 ████ |
██  ██|
██  ██|
██████|
 ████ |
"""));
        digitDataDoubleSized.put('9', convertStringToBooleanArray(
"""
 ████ |
██████|
██  ██|
██  ██|
██████|
 █████|
    ██|
    ██|
██████|
█████ |
"""));
    }

    private static boolean[] convertStringToBooleanArray(String string) {
        boolean[] b = new boolean[string.length()];
        int i = 0;
        for (String line : string.split("\n")) {
            for (char ch : line.toCharArray()) {
                if(ch == '|') {break;}
                b[i++] = ch == '█';
            }

        }
        return b;
    }
    @Override
    public String run(SpriteUtilsArgs args) {
        if (args == null) {
            LOG.error("SpriteUtilsArgs cannot be null.");
            throw new IllegalArgumentException("SpriteUtilsArgs cannot be null.");
        }
        SpriteUtilsOptions spriteUtilsOptions = new SpriteUtilsOptions(args);

        File workingDirectory = new File(spriteUtilsOptions.getWorkingDirectory());
        System.out.println("Going to process images in directory: " + workingDirectory);
        for (File imageFile : workingDirectory.listFiles()) {
            if (spriteUtilsOptions.getFileName().isPresent() && !spriteUtilsOptions.getFileName().get().equals(imageFile.getName())) {
                continue;
            }
            if(imageFile.getName().endsWith(".backup")) {
                continue;
            }

            File backupFile = new File(imageFile.getAbsolutePath() + ".backup");
            try {
                if (backupFile.exists()) {
                    imageFile.delete();
                    Utils.copyFile(backupFile, imageFile);
                } else {
                    Utils.copyFile(imageFile, backupFile);
                }
            } catch (SpriteUtilsException e) {
                LOG.error("Error managing backup files", e);
                throw new SpriteUtilsException("Error managing backup files", e);
            }

            BMPImage image = null;
            try {
                image = BMPDecoder.readExt(imageFile);
            } catch (IOException e) {
                LOG.error("Reading image failed: {}", imageFile.getAbsolutePath(), e);
                throw new SpriteUtilsException("Reading image failed", e);
            }
            BufferedImage bi = image.getImage();
            

            Supplier<Integer> randomByte = () -> random.apply(0, 255);

            
            SpriteSheet spriteSheet = new SpriteSheet(new File(spriteUtilsOptions.getSpriteSheetPath()));
            final List<SpriteSheetRow> spriteSheetRows = spriteSheet
                    .getSpriteSheetRows(imageFile.getName().toLowerCase());
            Graphics2D g = bi.createGraphics();
            Stroke dashedStroke = configureGraphics(g);
            if(spriteSheetRows == null) {
                continue;
            }
            spriteSheetRows
                    .stream()
                    .filter(s -> spriteUtilsOptions.getRow().isEmpty() ? true : (spriteUtilsOptions.getRow().get() == s.getRow()))
                    .forEach(row -> {
                drawSpriteSheetRow(row, g, dashedStroke, spriteUtilsOptions);
            });

            g.dispose();
            try {
                BMPEncoder.write(bi, imageFile);
            } catch (IOException e) {
                LOG.error("Writing image failed: {}", imageFile.getAbsolutePath(), e);
                throw new SpriteUtilsException("Writing image failed", e);
            }
            LOG.info("Image details - Colour Count: {}, Colour Depth: {}, Height: {}, Width: {}, Indexed: {}",
                    image.getColourCount(), image.getColourDepth(), image.getHeight(), image.getWidth(), image.isIndexed());
            if (spriteUtilsOptions.getFileName().isPresent() && spriteUtilsOptions.getFileName().equals(imageFile.getName())) {
                break;
            }
        }

        return "";

    }

    private void drawSpriteSheetRow(SpriteSheetRow row, Graphics2D g, Stroke dashedStroke, SpriteUtilsOptions spriteUtilsOptions) {
        int startX = row.getX();
        int endX = startX + row.getWidth() - 1;
        int startY = row.getY();
        int endY = startY + row.getHeight() - 1;
        
        boolean numberDoubleSized = spriteUtilsOptions.isNumberDoubleSized();
        if(numberDoubleSized) {
            if(row.getWidth() < 23) {
                numberDoubleSized = false;
            }
        }
        
        if(spriteUtilsOptions.isDrawNumberEnabled()) {
            drawNumber(row.getNumberPerSheet(), g, endX - 2, endY - 1, numberDoubleSized, spriteUtilsOptions);
        }

        g.setStroke(dashedStroke);

        Color currentColor = g.getColor();
        g.setColor(spriteUtilsOptions.getRectangleColor());
        g.drawRect(row.getX(), row.getY(), row.getWidth() - 1, row.getHeight() - 1);
        g.setColor(currentColor);
    }

    private Stroke configureGraphics(Graphics2D g) {
        g.setColor(Color.black);
        Stroke originalStroke = g.getStroke();
        if(false)
        {
        return originalStroke;
        }
        Stroke dashedStroke = new BasicStroke(1, BasicStroke.CAP_SQUARE, BasicStroke.CAP_SQUARE,
                0, new float[]{1, 3}, 0);
        g.setStroke(dashedStroke);
        //g.drawRect(0,0, 28,19);

        return dashedStroke;
    }

    @AllArgsConstructor
    static class Point {

        int x;
        int y;

        void addToX(int n) {
            x = x + n;
        }

        void addToY(int n) {
            y = y + n;
        }

        Point createClone() {
            return new Point(x, y);
        }
    }

    private void drawNumber(int number, Graphics2D g, int endX, int endY, boolean doubleSize, SpriteUtilsOptions spriteUtilsOptions) {
        LOG.debug("Drawing number {}", number);
        Point end = new Point(endX, endY);
        String numberString = String.valueOf(number);

        if (!doubleSize && spriteUtilsOptions.isDrawNumberBackgroundEnabled()) {
            Color oldColor = g.getColor();
            g.setColor(Color.WHITE);
            g.fillRect(endX + 1, endY - 5 * (doubleSize ? 2 : 1), 1, 6 * (doubleSize ? 2 : 1));
            g.setColor(oldColor);
        }

        for (int i = 0; i < numberString.length(); i++) {
            char digit = numberString.charAt(numberString.length() - 1 - i);
            drawANumber(Character.getNumericValue(digit), g, end.x - (i * 4 * (doubleSize ? 2 : 1)), end.y, doubleSize, spriteUtilsOptions);
        }
    }

    private void drawANumber(int number, Graphics2D g, int endX, int endY, boolean doubleSize, SpriteUtilsOptions spriteUtilsOptions) {
        Point end = new Point(endX, endY);
        var data0 = doubleSize ? digitDataDoubleSized : digitData;
        boolean[] data = data0.get((char) ('0' + number));
        if (data == null) {
            throw new SpriteUtilsException("Character is not supported: " + number);
        }

        Point start = end.createClone();
        start.addToX((doubleSize ? -1 : 0) -2 * (doubleSize ? 2 : 1));
        start.addToY((doubleSize ? -2 : 0) -4 * (doubleSize ? 2 : 1));
        Color currentColor = g.getColor();
        if(spriteUtilsOptions.isDrawNumberBackgroundEnabled()) {
        g.setColor(Color.WHITE);
        g.fillRect(start.x - 1, start.y - 1, 4 * (doubleSize ? 2 : 1), 6 * (doubleSize ? 2 : 1));
        g.setColor(currentColor);
        }
        g.setColor(spriteUtilsOptions.isDrawNumberBackgroundEnabled() ? Color.BLACK : Color.YELLOW);
        drawANumber(g, start, data, doubleSize);
        g.setColor(currentColor);
    }

    private void drawANumber(Graphics2D g, Point start, boolean[] data, boolean doubleSize) {
        BiConsumer<Integer, Integer> drawPixel = (x, y) -> g.fillRect(start.x + x, start.y + y, 1, 1);

        int i = 0;
        for (int y = 0; y < (5 * (doubleSize ? 2 : 1)); y++) {
            for (int x = 0; x < (3 * (doubleSize ? 2 : 1)); x++) {
                if (data[i++]) {
                    drawPixel.accept(x, y);
                }
            }
        }
    }

}
