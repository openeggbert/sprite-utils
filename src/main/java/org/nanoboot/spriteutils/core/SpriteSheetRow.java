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
import java.util.Optional;
import lombok.Data;
import lombok.ToString;

/**
 * Represents a row in a sprite sheet.
 * This class is responsible for parsing and serializing sprite sheet rows from/to CSV format.
 * 
 * @author robertvokac
 */
@Data
@ToString
public class SpriteSheetRow {
    private static final String DELIMITER = ";";
    private static final String ID_DELIMITER = "__";
    private static final int MINIMUM_COLUMNS = 10;
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
   /**
     * Constructor to create a SpriteSheetRow from a CSV line.
     * 
     * @param csvLine the CSV line containing the sprite sheet row data
      @throws IllegalArgumentException if the CSV line does not contain enough columns or contains invalid data
     */
    public SpriteSheetRow(String csvLine) {
        String[] csvColumns = csvLine.split(DELIMITER);
        if (csvColumns.length < MINIMUM_COLUMNS) {
            throw new IllegalArgumentException("CSV line does not contain enough columns: " + csvLine);
        }
        try {
            int i = 0;
            file = csvColumns[i++];
            group = csvColumns[i++];
            numberInGroup = Integer.parseInt(csvColumns[i++]);
            row = Integer.parseInt(csvColumns[i++]);
            column = Integer.parseInt(csvColumns[i++]);
            x = parseOptionalInt(csvColumns[i++]).orElse(-1);
            y = Integer.parseInt(csvColumns[i++]);
            width = Integer.parseInt(csvColumns[i++]);
            var column_ = csvColumns[i++];
            height = (column_.length() == 0 || column_.equals("0")) ? 0 :Integer.parseInt(column_);
            notes = i < csvColumns.length ? csvColumns[i++] : "";
            tags = i < csvColumns.length ? csvColumns[i++] : "";
            numberPerSheet = i < csvColumns.length ? Integer.parseInt(csvColumns[i]) : 0;

            // Adjust height if the column is greater than 1
            if (column > 1) {
                height = -height;
            }
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("CSV line contains invalid number format: " + csvLine, e);
        }
    }
    /**
     * Creates an ID for the sprite sheet row.
     * 
     * @return the generated ID
     */
    public String createId() {
        return String.join(ID_DELIMITER, file, group, String.valueOf(numberInGroup));
    }
    /**
     * Converts the sprite sheet row to a CSV line.
     * 
     * @return the CSV line representation of the sprite sheet row
     */
    public String toCsvLine() {
        List<String> fields = Arrays.asList(
            file, group, String.valueOf(numberInGroup), String.valueOf(row),
            String.valueOf(column), String.valueOf(x), String.valueOf(y),
            String.valueOf(width), String.valueOf(height), notes, tags,
            String.valueOf(numberPerSheet)
        );
        return String.join(DELIMITER, fields);
    }
    /**
     * Parses an optional integer from a string.
     * 
     * @param value the string to parse
     * @return an Optional containing the parsed integer, or empty if the string is blank
     */
    private Optional<Integer> parseOptionalInt(String value) {
        try {
            return value.isBlank() ? Optional.empty() : Optional.of(Integer.parseInt(value));
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid integer format: " + value, e);
        }
    }
}
