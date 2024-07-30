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

import java.util.Arrays;
import java.util.List;
import lombok.Data;
import lombok.ToString;

/**
 *
 * @author robertvokac
 */
@Data
@ToString
public class SpriteSheetRow {

    String file;
    String group;
    int numberInGroup;
    int row;
    int column;
    int x;
    int y;
    int width;
    int height;
    String notes;
    String tags;
    int numberPerSheet;

    public SpriteSheetRow(String csvLine) {
        String[] csvColumn = csvLine.split(";");
        if (csvColumn.length < 10) {
            throw new IllegalArgumentException("CSV line does not contain enough (10) columns.");
        }
        int i = 0;
        file = csvColumn[i++];
        group = csvColumn[i++];
        numberInGroup = Integer.parseInt(csvColumn[i++]);
        row = Integer.parseInt(csvColumn[i++]);
        column = Integer.parseInt(csvColumn[i++]);
        x = csvColumn[i++].isBlank() ? -1 : Integer.parseInt(csvColumn[i - 1]);
        y = Integer.parseInt(csvColumn[i++]);
        width = Integer.parseInt(csvColumn[i++]);
        height = Integer.parseInt(csvColumn[i++]);
        if (column > 1) {
            height = -height;
        }
        notes = csvColumn.length >= 10 ? csvColumn[i++] : "";
        tags = csvColumn.length > 11 ? csvColumn[i++] : "";
    }

    public String createId() {
        return file + __ + group + __ + numberInGroup;
    }
    private static final String __ = "__";
    private static final String DELIMITER = ";";

    public String toCsvLine() {
        List<String> fields = Arrays.asList(
                file,
                group,
                String.valueOf(numberInGroup),
                String.valueOf(row),
                String.valueOf(column),
                String.valueOf(x),
                String.valueOf(y),
                String.valueOf(width),
                String.valueOf(height),
                notes,
                tags,
                String.valueOf(numberPerSheet)
        );
        return String.join(DELIMITER, fields);
    }

}
