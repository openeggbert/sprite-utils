///////////////////////////////////////////////////////////////////////////////////////////////
// sprite-utils: Tool used to work with sprites
// Copyright (C) 2023-2023 the original author or authors.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; version 2
// of the License only.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
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
        int i = (int) (Math.random() * diff);
        return min + i;
    };

    @Override
    public String run(SpriteUtilsArgs bitBackupArgs) {
        File workingDirectory = new File("/home/robertvokac/Desktop/speedy_eggbert/speedy_eggbert_I/IMAGE08");
        File testFile = new File(workingDirectory, "blupi000.blp");
        File backupFile = new File(testFile.getAbsolutePath() + ".backup");
        if (backupFile.exists()) {
            testFile.delete();
            Utils.copyFile(backupFile, testFile);
        } else {
            Utils.copyFile(testFile, backupFile);
        }

        BMPImage image = null;
        File file = testFile;
        try {
            image = BMPDecoder.readExt(file);
        } catch (IOException ex) {
            throw new SpriteUtilsException("Reading image failed: " + file.getAbsolutePath() + " " + ex.getMessage());
        }
        BufferedImage bi = image.getImage();
        Graphics2D g = bi.createGraphics();

        Supplier<Integer> randomByte = () -> random.apply(0, 255);

        final int height = image.getHeight();
        final int width = image.getWidth();
        g.setColor(Color.red);
        Stroke originalStroke = g.getStroke();
        Stroke dashedStroke = new BasicStroke(1, BasicStroke.CAP_SQUARE, BasicStroke.CAP_SQUARE,
                0, new float[]{1, 3}, 0);
        g.setStroke(dashedStroke);
        //g.drawRect(0,0, 28,19);
        SpriteSheet spriteSheet = new SpriteSheet(new File(workingDirectory, "spritesheet.csv"));
        spriteSheet.getSpriteSheets(testFile.getName()).forEach(r -> {
            int startX = r.getX();
            int endX = startX + r.getWidth();
            int startY = r.getY();
            int endY = startY + r.getHeight();
            drawNumber(r.getNumberPerSheet(), g, endX - 2, endY - 1);
            
//            
//            int x = 0;
//            int y = 0;
//            
//            int aaa = 3;
//            x = startX;y = startY;
//            g.setStroke(originalStroke);
//            g.drawLine(x, y, x + aaa, y);
//            g.drawLine(x, y, x, y + aaa);
//            
//            x = endX;y = startY;
//            g.drawLine(x, y, x - aaa, y);
//            g.drawLine(x, y, x, y + aaa);
//            
//            x = startX;y = endY;
//            g.drawLine(x, y, x + aaa, y);
//            g.drawLine(x, y, x, y - aaa);
//            
//            x = endX;y = endY;
//            g.drawLine(x, y, x - aaa, y);
//            g.drawLine(x, y, x, y - aaa);

            g.setStroke(dashedStroke);

            g.drawRect(r.getX(), r.getY(), r.getWidth(), r.getHeight());
        });
//        for (int i = 0; i <= 100; i++) {
//            g.setStroke(new BasicStroke(random.apply(1,3)));
//
//            g.setColor(new Color(randomByte.get(), randomByte.get(), randomByte.get()));
//
//            int h = random.apply(10, 100);
//            int w = random.apply(10, 100);
//            int x = random.apply(1, width);
//            int y = random.apply(1, height);
//            if ((x + w) > width) {
//                x = x - w - random.apply(5, 25);
//            }
//            if ((y + h) > height) {
//                y = y - h - random.apply(5, 25);
//            }
//            g.drawRect(x, y, w, h);
//        }
        g.dispose();
        try {
            BMPEncoder.write(bi, testFile);
        } catch (IOException ex) {
            throw new SpriteUtilsException("Writing image failed: " + file.getAbsolutePath() + " " + ex.getMessage());
        }
        System.out.println("getColourCount=" + image.getColourCount());
        System.out.println("getColourDepth=" + image.getColourDepth());
        System.out.println("getHeight=" + height);
        System.out.println("getWidth=" + width);
        System.out.println("isIndexed=" + image.isIndexed());
        return "";

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

    private static void drawNumber(int number, Graphics2D g, int endX, int endY) {
        //number = random.apply(100, 900);
        System.out.println("number = " + number);
        Point end = new Point(endX, endY);
        String ns = String.valueOf(number);
        
        Color oldColor = g.getColor();
        g.setColor(Color.WHITE);
        g.fillRect(endX + 1, endY - 5, 1, 6);
        g.setColor(oldColor);
        
        
        for (int i = 0; i < ns.length(); i++) {
            char ch = ns.charAt(ns.length() - 1 - i);
            String s = String.valueOf(ch);
            drawANumber(Integer.valueOf(s), g, end.x - (i * 4), end.y);

        }
    }

         private static final boolean[] data0 = new boolean[]{
                true, true, true,
                true, false, true,
                true, false, true,
                true, false, true,
                true, true, true};
           private static final boolean[] data1 = new boolean[]{
                false, false, true,
                false, true, true,
                true, false, true,
                false, false, true,
                false, false, true};
          private static final boolean[] data2 = new boolean[]{
                true, true, true,
                false, false, true,
                false, true, false,
                true, false, false,
                true, true, true};
         private static final boolean[] data3 = new boolean[]{
                true, true, true,
                false, false, true,
                true, true, true,
                false, false, true,
                true, true, true};
          private static final boolean[] data4 = new boolean[]{
                false, false, true,
                false, true, false,
                true, true, true,
                false, false, true,
                false, false, true};
        private static final boolean[] data5 = new boolean[]{
                true, true, true,
                true, false, false,
                true, true, true,
                false, false, true,
                true, true, true};
         private static final boolean[] data6 = new boolean[]{
                true, true, true,
                true, false, false,
                true, true, true,
                true, false, true,
                true, true, true};
        private static final boolean[] data7 = new boolean[]{
                true, true, true,
                false, false, true,
                false, false, true,
                false, false, true,
                false, false, true};
         private static final boolean[] data8 = new boolean[]{
                true, true, true,
                true, false, true,
                true, true, true,
                true, false, true,
                true, true, true};
        private static final boolean[] data9 = new boolean[]{
                true, true, true,
                true, false, true,
                true, true, true,
                false, false, true,
                false, false, true};
        
    private static void drawANumber(int number, Graphics2D g, int endX, int endY) {
        System.out.println("drawNumber " +number + " " + endX + " " + endY);
        Point end = new Point(endX, endY);
        String ns = String.valueOf(number);
char c = ns.charAt(0);
        Point start = end.createClone();
Point p = start;
        start.addToX(-2);
        start.addToY(-4);
        Color oldColor = g.getColor();
        g.setColor(Color.WHITE);
        g.fillRect(start.x-1, start.y-1, 4, 6);
//        if (true) {
//            g.setColor(oldColor);
//            return;
//        }
        g.setColor(Color.BLACK);


            switch (c) {
                case '0':
                    drawANumber(g, p, data0);
                    break;
                case '1':
                    drawANumber(g, p, data1);
                    break;
                case '2':
                    drawANumber(g, p, data2);
                    break;
                case '3':
                    drawANumber(g, p, data3);
                    break;
                case '4':
                    drawANumber(g, p, data4);
                    break;
                case '5':
                    drawANumber(g, p, data5);
                    break;
                case '6':
                    drawANumber(g, p, data6);
                    break;
                case '7':
                    drawANumber(g, p, data7);
                    break;
                case '8':
                    drawANumber(g, p, data8);
                    break;
                case '9':
                    drawANumber(g, p, data9);
                    break;
                default:
                    throw new SpriteUtilsException("Character is not supported: " + c);
            }

    
        g.setColor(oldColor);
    }

    private static void drawANumber(Graphics2D g, Point start, boolean[] data) {
        
        BiConsumer<Integer, Integer> drawPixel = (x, y) -> 
        {
            System.out.println("Drawing pixel: " + x + " " + y);
            //g.setColor(Color.ORANGE);
            g.fillRect(start.x + x, start.y + y, 1, 1);
            //g.drawLine(start.x + x, start.y + y, start.x + x, start.y + y);
        };
        //drawPixel.accept(1,1);
        //if(true) return;
        int i = 0;
        int X = 0;
        int Y = 0;
        
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X = 0;
        Y++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X = 0;
        Y++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X = 0;
        Y++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X = 0;
        Y++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X++;
        if (data[i++]) {
            drawPixel.accept(X, Y);
        }
        X = 0;
        Y++;

    }
}
