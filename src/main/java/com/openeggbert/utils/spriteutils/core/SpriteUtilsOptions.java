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
package com.openeggbert.utils.spriteutils.core;

import java.awt.Color;
import java.util.Optional;

/**
 *
 * @author robertvokac
 */
public class SpriteUtilsOptions {

    private final SpriteUtilsArgs spriteUtilsArgs;

    public SpriteUtilsOptions(SpriteUtilsArgs spriteUtilsArgs) {
        this.spriteUtilsArgs = spriteUtilsArgs;
    }

    public String getWorkingDirectory() {
        return spriteUtilsArgs.getArgumentOptional("--dir").orElse(".");
    }

    public boolean isDrawNumberEnabled() {
        return spriteUtilsArgs.getOptionalBooleanArgument("--draw-number", true);
    }
    public boolean isDrawNumberBackgroundEnabled() {
        return spriteUtilsArgs.getOptionalBooleanArgument("--draw-number-background", true);
    }
    public boolean isNumberDoubleSized() {
        return spriteUtilsArgs.getBooleanArgument("--double-sized-number");
    }
    public Color getRectangleColor() {
        Optional<String> arg = spriteUtilsArgs.getArgumentOptional("--rectangle-color");
        if (arg.isEmpty()) {
            return Color.RED;
        }
        String[] array = arg.get().split(",");
        if (array.length != 3) {
            throw new SpriteUtilsException("Invalid format of rectangle-color option: " + arg.get());
        }
        return new Color(
                Integer.parseInt(array[0]),
                Integer.parseInt(array[1]),
                Integer.parseInt(array[2])
        );
    }

    public String getSpriteSheetPath() {
        return spriteUtilsArgs.getArgumentOptional("--sprite-sheet-path").orElse(getWorkingDirectory() + "/spritesheet.csv");
    }
    
    
    public Optional<String> getFileName() {
        return spriteUtilsArgs.getArgumentOptional("--file-name");
    }
    public Optional<Integer> getRow() {
        Optional<String> arg = spriteUtilsArgs.getArgumentOptional("--row");
        return arg.isEmpty() ? Optional.empty() : Optional.of(Integer.valueOf(arg.get()));
    }

}
