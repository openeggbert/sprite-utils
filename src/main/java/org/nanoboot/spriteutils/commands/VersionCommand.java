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
package org.nanoboot.spriteutils.commands;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.nanoboot.spriteutils.core.Command;
import org.nanoboot.spriteutils.core.SpriteUtilsArgs;

/**
 *
 * @author <a href="mailto:mail@robertvokac.com">Robert Vokac</a>
 */
public class VersionCommand implements Command {

    public static final String NAME = "version";
    private static final Logger LOG = LogManager.getLogger(VersionCommand.class);

    public VersionCommand() {
    }

    @Override
    public String getName() {
        return NAME;
    }

    @Override
    public String run(SpriteUtilsArgs bitInspectorArgs) {
        String result = "Sprite Utils 0.0.0-SNAPSHOT";
        LOG.info(result);
        return result;
    }

}
