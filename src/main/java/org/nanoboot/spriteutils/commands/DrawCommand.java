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

    static {
        digitData.put('0', new boolean[]{
            true, true, true,
            true, false, true,
            true, false, true,
            true, false, true,
            true, true, true});
        digitData.put('1', new boolean[]{
            false, false, true,
            false, true, true,
            true, false, true,
            false, false, true,
            false, false, true});
        digitData.put('2', new boolean[]{
            true, true, true,
            false, false, true,
            false, true, false,
            true, false, false,
            true, true, true});
        digitData.put('3', new boolean[]{
            true, true, true,
            false, false, true,
            true, true, true,
            false, false, true,
            true, true, true});
        digitData.put('4', new boolean[]{
            false, false, true,
            false, true, false,
            true, true, true,
            false, false, true,
            false, false, true});
        digitData.put('5', new boolean[]{
            true, true, true,
            true, false, false,
            true, true, true,
            false, false, true,
            true, true, true});
        digitData.put('6', new boolean[]{
            true, true, true,
            true, false, false,
            true, true, true,
            true, false, true,
            true, true, true});
        digitData.put('7', new boolean[]{
            true, true, true,
            false, false, true,
            false, false, true,
            false, false, true,
            false, false, true});
        digitData.put('8', new boolean[]{
            true, true, true,
            true, false, true,
            true, true, true,
            true, false, true,
            true, true, true});
        digitData.put('9', new boolean[]{
            true, true, true,
            true, false, true,
            true, true, true,
            false, false, true,
            true, true, true});
    }

    @Override
    public String run(SpriteUtilsArgs args) {
        if (args == null) {
            LOG.error("SpriteUtilsArgs cannot be null.");
            throw new IllegalArgumentException("SpriteUtilsArgs cannot be null.");
        }

        File workingDirectory = new File("/rv/data/desktop/code/code.nanoboot.org/nanoboot/open-eggbert-data/Speedy_Eggbert_1/Game/IMAGE08");
        File testFile = new File(workingDirectory, "BLUPI000.BLP");
        File backupFile = new File(testFile.getAbsolutePath() + ".backup");
        try {
            if (backupFile.exists()) {
                testFile.delete();
                Utils.copyFile(backupFile, testFile);
            } else {
                Utils.copyFile(testFile, backupFile);
            }
        } catch (SpriteUtilsException e) {
            LOG.error("Error managing backup files", e);
            throw new SpriteUtilsException("Error managing backup files", e);
        }

        BMPImage image = null;
        try {
            image = BMPDecoder.readExt(testFile);
        } catch (IOException e) {
            LOG.error("Reading image failed: {}", testFile.getAbsolutePath(), e);
            throw new SpriteUtilsException("Reading image failed", e);
        }
        BufferedImage bi = image.getImage();
        Graphics2D g = bi.createGraphics();

        Supplier<Integer> randomByte = () -> random.apply(0, 255);

        Stroke dashedStroke = configureGraphics(g);
        SpriteSheet spriteSheet = new SpriteSheet(new File(workingDirectory, "spritesheet.csv"));
        spriteSheet.getSpriteSheets(testFile.getName().toLowerCase()).forEach(row -> {
            drawSpriteSheetRow(row, g, dashedStroke);
        });

        g.dispose();
        try {
            BMPEncoder.write(bi, testFile);
        } catch (IOException e) {
            LOG.error("Writing image failed: {}", testFile.getAbsolutePath(), e);
            throw new SpriteUtilsException("Writing image failed", e);
        }
        LOG.info("Image details - Colour Count: {}, Colour Depth: {}, Height: {}, Width: {}, Indexed: {}",
                image.getColourCount(), image.getColourDepth(), image.getHeight(), image.getWidth(), image.isIndexed());
        return "";

    }

    private void drawSpriteSheetRow(SpriteSheetRow row, Graphics2D g, Stroke dashedStroke) {
        int startX = row.getX();
        int endX = startX + row.getWidth() - 1;
        int startY = row.getY();
        int endY = startY + row.getHeight() - 1;
        drawNumber(row.getNumberPerSheet(), g, endX - 2, endY - 1);

        g.setStroke(dashedStroke);

        Color currentColor = g.getColor();
        g.setColor(Color.RED);
        g.drawRect(row.getX(), row.getY(), row.getWidth() - 1, row.getHeight() - 1);
        g.setColor(currentColor);
    }

    private Stroke configureGraphics(Graphics2D g) {
        g.setColor(Color.black);
        Stroke originalStroke = g.getStroke();
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

    private void drawNumber(int number, Graphics2D g, int endX, int endY) {
        LOG.debug("Drawing number {}", number);
        Point end = new Point(endX, endY);
        String numberString = String.valueOf(number);

        Color oldColor = g.getColor();
        g.setColor(Color.WHITE);
        g.fillRect(endX + 1, endY - 5, 1, 6);
        g.setColor(oldColor);

        for (int i = 0; i < numberString.length(); i++) {
            char digit = numberString.charAt(numberString.length() - 1 - i);
            drawANumber(Character.getNumericValue(digit), g, end.x - (i * 4), end.y);
        }
    }

    private void drawANumber(int number, Graphics2D g, int endX, int endY) {
        Point end = new Point(endX, endY);
        boolean[] data = digitData.get((char) ('0' + number));
        if (data == null) {
            throw new SpriteUtilsException("Character is not supported: " + number);
        }

        Point start = end.createClone();
        start.addToX(-2);
        start.addToY(-4);
        Color currentColor = g.getColor();
        g.setColor(Color.WHITE);
        g.fillRect(start.x - 1, start.y - 1, 4, 6);
        g.setColor(currentColor);
        drawANumber(g, start, data);
    }

    private void drawANumber(Graphics2D g, Point start, boolean[] data) {

        BiConsumer<Integer, Integer> drawPixel = (x, y) -> g.fillRect(start.x + x, start.y + y, 1, 1);

        int i = 0;
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 3; x++) {
                if (data[i++]) {
                    drawPixel.accept(x, y);
                }
            }
        }
    }

}
