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

import org.nanoboot.spriteutils.core.Command;
import org.nanoboot.spriteutils.core.SpriteUtilsArgs;

/**
 *
 * @author <a href="mailto:mail@robertvokac.com">Robert Vokac</a>
 */
public class HelpCommand implements Command {

    public static final String NAME = "help";

    public HelpCommand() {
    }

    @Override
    public String getName() {
        return NAME;
    }

    @Override
    public String run(SpriteUtilsArgs bitInspectorArgs) {
        String str = """
    NAME
        spriteutils - " Sprite Utils"
                           
    SYNOPSIS
        spriteutils [command] [options]
                           
    DESCRIPTION
        Tools used to work with sprites.
                           
    COMMAND
        draw        draw rectangles for sprites
                        OPTIONS
                            color={rgb value of the rectangle border}
                                Optional. Default=255,0,0
                            files={comma separated list of BMP files in the working directory}
                                Optional. Default=(all BMP files in the working directory).
                            groups={comma separated list of sprite groups}
                                Optional. Default=(all sprite groups).
                            positon={row starting with 0, height starting with 0}
                                Optional. Default=(all sprites).
                            number-per-group={row starting with 0, height starting with 0}
                                Optional. Default=(all sprites).
        help        Display help information
        version     Display version information                           
""";
        System.out.println(str);
        return str;
    }

}
