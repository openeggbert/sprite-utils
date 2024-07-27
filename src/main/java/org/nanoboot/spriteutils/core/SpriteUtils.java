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

package org.nanoboot.spriteutils.core;

import java.util.HashSet;
import java.util.Set;
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
    
    private final Set<Command> commandImplementations;
    public SpriteUtils() {
        commandImplementations = new HashSet<>();
        commandImplementations.add(new DrawCommand());
        commandImplementations.add(new HelpCommand());
        commandImplementations.add(new VersionCommand());
    }
       
    public void run(String[] args) {
        run(new SpriteUtilsArgs(args));
    }
    
    public void run(SpriteUtilsArgs spriteUtilsArgs) {
        String command = spriteUtilsArgs.getCommand();
        Command foundCommand = null;
        for(Command e:commandImplementations) {
            if(e.getName().equals(command)) {
                foundCommand = e;
                break;
            }
        }
        if(foundCommand == null) {
            String msg = "Command \"" + command + "\" is not supported.";
            LOG.error(msg);
            
            new HelpCommand().run(spriteUtilsArgs);
            throw new SpriteUtilsException(msg);
        }
        foundCommand.run(spriteUtilsArgs);
        
    }
}
