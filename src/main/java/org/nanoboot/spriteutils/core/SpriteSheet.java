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
package org.nanoboot.spriteutils.core;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;
import lombok.Data;
import lombok.ToString;

/**
 * Represents a sprite sheet and handles parsing from a CSV file.
 * This class maintains a map of sprite sheet rows and processes each row according to specified rules.
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
    /**
     * Constructor to create a SpriteSheet from a CSV file.
     * 
     * @param file the CSV file containing sprite sheet data
     */
    public SpriteSheet(File file) {
        List<SpriteSheetRow> rows = new ArrayList<>();
        String text = Utils.readTextFromFile(file);
        text.lines().skip(1).takeWhile(line -> !line.contains("skipskip"))
                .forEach(line -> {
                    processLine(line, rows);
                });
        saveComputedFile(file, text, rows);

    }

    private void saveComputedFile(File file, String text, List<SpriteSheetRow> rows) {
        File computed = new File(file.getAbsolutePath() + ".computed.csv");
        if (computed.exists()) {
            computed.delete();
        }
        StringBuilder sb = new StringBuilder(text.lines().limit(1).findFirst().orElse(""));
        sb.append("\n");
        rows.stream().forEach(r -> {
            sb.append(r.toCsvLine()).append("\n");
        });
        Utils.writeTextToFile(sb.toString(), computed);
    }

    private void processLine(String line, List<SpriteSheetRow> rows) throws SpriteUtilsException {
        final SpriteSheetRow spriteSheetRow = new SpriteSheetRow(line);

        validateRow(spriteSheetRow);

        SpriteSheetRow previousSheetRow = rows.isEmpty() ? null : rows.get(rows.size() - 1);

        if (spriteSheetRow.getColumn() > 1 && previousSheetRow != null && Math.abs(spriteSheetRow.height) >= Math.abs(previousSheetRow.height) && (spriteSheetRow.getColumn() > 2 ? (previousSheetRow.height != 0 && spriteSheetRow.height != 0) : false)) {
            spriteSheetRow.height = -spriteSheetRow.height;
        }

        updateSpriteSheetRow(spriteSheetRow);

        rows.add(spriteSheetRow);
        updateMap(spriteSheetRow);
        System.out.println(spriteSheetRow.toCsvLine());
    }

    private void updateSpriteSheetRow(final SpriteSheetRow spriteSheetRow) {
        if (spriteSheetRow.x == -1) {
            spriteSheetRow.x = (spriteSheetRow.column == 1) ? 0
                    : Optional.of(lastX == -1 ? null : (lastX + lastWidth)).orElseThrow(() -> new SpriteUtilsException("Could not compute X for " + spriteSheetRow.createId()));
        }
        lastX = spriteSheetRow.getX();
        lastWidth = spriteSheetRow.getWidth();

        if (spriteSheetRow.height <= 0) {
            spriteSheetRow.height = Optional.of((lastHeight != -1) ? (lastHeight + (-spriteSheetRow.height)) : null).orElseThrow(() -> new SpriteUtilsException("Could not compute height for " + spriteSheetRow.createId()));
        }
        lastHeight = spriteSheetRow.height;
        lastSpriteSheetRow = spriteSheetRow;
    }

    private void updateMap(final SpriteSheetRow spriteSheetRow) {
        if (!map.containsKey(spriteSheetRow.file)) {
            map.put(spriteSheetRow.file, new ArrayList<>());
        }
        map.get(spriteSheetRow.file).add(spriteSheetRow);
    }

    private void validateRow(final SpriteSheetRow spriteSheetRow) {
        if (lastSpriteSheetRow != null) {
            if (!spriteSheetRow.file.equals(lastSpriteSheetRow.file)) {
                lastSpriteSheetRow = null;
            }
        }
        if (lastSpriteSheetRow == null) {
            validateFirstRow(spriteSheetRow);

        } else {
            validateSubsequentRow(spriteSheetRow);
        }
    }

    private void validateSubsequentRow(SpriteSheetRow spriteSheetRow) {
        spriteSheetRow.numberPerSheet = lastSpriteSheetRow.numberPerSheet + 1;
        if (spriteSheetRow.row > lastSpriteSheetRow.row) {
            if (spriteSheetRow.row != (lastSpriteSheetRow.row + 1)) {
                throw new SpriteUtilsException("Unexpected row for " + spriteSheetRow.createId());
            }
        } else if (spriteSheetRow.row < lastSpriteSheetRow.row) {
            throw new SpriteUtilsException("Unexpected row for " + spriteSheetRow.createId());
        } else if (spriteSheetRow.column != (lastSpriteSheetRow.column + 1)) {
            throw new SpriteUtilsException("Unexpected column for " + spriteSheetRow.createId());
        }
    }

    private void validateFirstRow(SpriteSheetRow spriteSheetRow) {
        if (spriteSheetRow.getRow() != 1 || spriteSheetRow.getColumn() != 1) {
            throw new SpriteUtilsException("Invalid initial row or column for file " + spriteSheetRow.file);
        }
        spriteSheetRow.numberPerSheet = 1;
    }

    public List<SpriteSheetRow> getSpriteSheetRows(String file) {
        System.out.println("getSpriteSheetRows() file=" + file);
        return map.get(file);
    }
    public List<SpriteSheetRow> getSpriteSheetRows() {
        return map.keySet().stream().map(k -> map.get(k)).flatMap(List::stream).collect(Collectors.toList());
    }
}
