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

import java.util.HashMap;
import java.util.Map;
import lombok.Getter;

/**
 *
 * @author <a href="mailto:mail@robertvokac.com">Robert Vokac</a>
 */
public class SpriteUtilsArgs {

    @Getter
    private final String command;
    private final Map<String, String> internalMap = new HashMap<>();

    private static String[] convertToStringArray(String command, Map<String, String> map) {
        String[] array = new String[1 + map.size()];
        array[0] = command;
        int i = 0;
        for (String key : map.keySet()) {
            array[++i] = key + "=" + map.get(key);
        }
        return array;
    }

    public SpriteUtilsArgs(SpriteUtilsCommand command, Map<String, String> map) {
        this(convertToStringArray(command.name().toLowerCase(), map));
    }

    public SpriteUtilsArgs(String command, Map<String, String> map) {
        this(convertToStringArray(command, map));
    }

    public SpriteUtilsArgs(String[] args) {
        command = args.length == 0 ? "check" : args[0];

        if (args.length > 1) {

            for (String arg : args) {
                if (arg == null) {
                    continue;
                }
                if (args[0].equals(arg)) {
                    continue;
                }
                String[] keyValue = arg.split("=", 2);
                internalMap.put(keyValue[0], keyValue.length > 1 ? keyValue[1] : null);
            }
            for (String key : internalMap.keySet()) {
                System.out.println("Found argument: " + key + "(=)" + internalMap.get(key));
            }

        }
    }

    public boolean hasArgument(String arg) {
        return internalMap.containsKey(arg);
    }

    public void addArgument(String arg, String value) {
        this.internalMap.put(arg, value);
    }

    public String getArgument(String arg) {
        return internalMap.get(arg);
    }
    public boolean isVerboseLoggingEnabled() {
        return hasArgument("verbose")&&getArgument("verbose").equals("true");
    }

}
