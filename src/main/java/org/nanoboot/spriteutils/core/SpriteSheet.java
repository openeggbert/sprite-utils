/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package org.nanoboot.spriteutils.core;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import lombok.Data;
import lombok.ToString;

/**
 *
 * @author robertvokac
 */
@Data
@ToString
public class SpriteSheet {

    private final Map<String, List<SpriteSheetRow>> map = new HashMap<>();
    private static int lastX = -1;
    private static int lastWidth = -1;
    private static int lastHeight = -1;
    private SpriteSheetRow lastSpriteSheetRow = null;

    public SpriteSheet(File file) {
        List<SpriteSheetRow> rows = new ArrayList<>();
        String text = Utils.readTextFromFile(file);
        text.lines().skip(1).forEach(l -> {
            final SpriteSheetRow spriteSheetRow = new SpriteSheetRow(l);

            if (lastSpriteSheetRow != null) {
                if (!spriteSheetRow.file.equals(lastSpriteSheetRow.file)) {
                    lastSpriteSheetRow = null;
                }
            }
            if (lastSpriteSheetRow == null) {
                spriteSheetRow.numberPerSheet = 1;
                if (spriteSheetRow.getRow() != 1) {
                    String msg = "Row of the first sprite in file must be equal to 1, but value for file " + spriteSheetRow.file + " is " + spriteSheetRow.getRow();
                    throw new SpriteUtilsException(msg);
                }
                if (spriteSheetRow.getColumn() != 1) {
                    String msg = "Column of the first sprite in file must be equal to 1, but value for file " + spriteSheetRow.file + " is " + spriteSheetRow.getColumn();
                    throw new SpriteUtilsException(msg);
                }

            } else {
                spriteSheetRow.numberPerSheet = lastSpriteSheetRow.numberPerSheet + 1;

                if(spriteSheetRow.row > lastSpriteSheetRow.row) {
                    if(spriteSheetRow.row != lastSpriteSheetRow.row) {
                        throw new SpriteUtilsException("Unexpected row for " + spriteSheetRow.createId());
                    }
                }
                if(spriteSheetRow.row < lastSpriteSheetRow.row) {
                        throw new SpriteUtilsException("Unexpected row for " + spriteSheetRow.createId());
                }
                
                if(spriteSheetRow.row == lastSpriteSheetRow.row) {
                    if(spriteSheetRow.column != (lastSpriteSheetRow.column + 1))
                        throw new SpriteUtilsException("Unexpected column for " + spriteSheetRow.createId());
                }
            }
            rows.add(spriteSheetRow);
            if (!map.containsKey(spriteSheetRow.file)) {
                map.put(spriteSheetRow.file, new ArrayList<>());
            }
            map.get(spriteSheetRow.file).add(spriteSheetRow);
            if (spriteSheetRow.x == -1) {
                if (spriteSheetRow.column == 1) {
                    spriteSheetRow.x = 0;
                } else {
                    if (lastX == -1) {
                        throw new SpriteUtilsException("Could not compute X for " + spriteSheetRow.createId());
                    } else {
                        spriteSheetRow.x = lastX + lastWidth + 1;
                    }
                }
            }
            lastX = spriteSheetRow.getX();
            lastWidth = spriteSheetRow.getWidth();
            if (spriteSheetRow.height <= 0) {
                if (lastHeight == -1) {
                    throw new SpriteUtilsException("Could not compute height for " + spriteSheetRow.createId());
                } else {
                    spriteSheetRow.height = lastHeight + ((-1) * spriteSheetRow.height);
                }
            }
            lastHeight = spriteSheetRow.height;
            lastSpriteSheetRow = spriteSheetRow;
            System.out.println(spriteSheetRow.toCsvLine());
        });
        File computed = new File(file.getAbsolutePath() + ".computed.csv");
        if(computed.exists()){
            computed.delete();
        }
        StringBuilder sb = new StringBuilder(text.lines().limit(1).findFirst().orElse(""));
        sb.append("\n");
        rows.stream().forEach(r-> {
        sb.append(r.toCsvLine()).append("\n");
        });
        Utils.writeTextToFile(sb.toString(), computed);

    }

    public List<SpriteSheetRow> getSpriteSheets(String file) {
        return map.get(file);
    }

}
