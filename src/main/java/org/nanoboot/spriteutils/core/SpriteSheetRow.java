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
import java.util.stream.Collector;
import java.util.stream.Collectors;
import java.util.stream.Stream;
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
        int i = 0;
        file = csvColumn[i++];
        group = csvColumn[i++];
        numberInGroup = Integer.valueOf(csvColumn[i++]);
        row = Integer.valueOf(csvColumn[i++]);
        column = Integer.valueOf(csvColumn[i++]);
        String xString = csvColumn[i++];
        x = xString.isBlank() ? -1 : Integer.valueOf(xString);
        y = Integer.valueOf(csvColumn[i++]);
        width = Integer.valueOf(csvColumn[i++]);
        height = Integer.valueOf(csvColumn[i++]);
        if (column > 1) {
            height = height * (-1);
        }
        notes = csvColumn.length >= 10 ? csvColumn[i++] : "";
        tags = csvColumn.length > 11 ? csvColumn[i++] : "";
    }

    public String createId() {
        return file + __ + group + __ + numberInGroup;
    }
    private static final String __ = "__";

    public String toCsvLine() {
        List<String> l = Stream.of(
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
                String.valueOf(numberPerSheet)).toList();
        return l.stream().collect(Collectors.joining(";"));

    }

}
