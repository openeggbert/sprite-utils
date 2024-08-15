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

import java.util.HashMap;
import java.util.Map;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.nanoboot.spriteutils.commands.DrawCommand;
import org.nanoboot.spriteutils.commands.HelpCommand;
import org.nanoboot.spriteutils.commands.VersionCommand;

/**
 *
 * @author <a href="mailto:mail@robertvokac.com">Robert Vokac</a>
 */
public class SpriteUtils {

    private static final Logger LOG = LogManager.getLogger(SpriteUtils.class);

    private final Map<String, Command> commandImplementations;

    public SpriteUtils() {
        commandImplementations = new HashMap<>();
        commandImplementations.put("draw", new DrawCommand());
        commandImplementations.put("help", new HelpCommand());
        commandImplementations.put("version", new VersionCommand());
    }

    public void run(String[] args) {
        //args = "draw --dir /rv/data/desktop/code/code.nanoboot.org/nanoboot/open-eggbert-legacy-assets/speedy_eggbert_1/IMAGE08 --rectangle-color 0,255,0 --sprite-sheet-path /rv/data/desktop/code/code.nanoboot.org/nanoboot/open-eggbert/assets/default-spritesheets/speedy_blupi_I.spritesheet.csv --row 2 --file-name OBJECT.BLP ".split(" ");
        run(new SpriteUtilsArgs(args));
    }

    public void run(SpriteUtilsArgs spriteUtilsArgs) {
        if (spriteUtilsArgs == null) {
            String msg = "No arguments provided.";
            LOG.error(msg);
            throw new IllegalArgumentException(msg);
        }
        String commandName = spriteUtilsArgs.getCommand();
        Command command = commandImplementations.get(commandName);
        if (command == null) {
            String msg = "Command \"" + commandName + "\" is not supported.";
            LOG.error(msg);
            new HelpCommand().run(spriteUtilsArgs);
            throw new SpriteUtilsException(msg);
        }

        command.run(spriteUtilsArgs);

    }
}
