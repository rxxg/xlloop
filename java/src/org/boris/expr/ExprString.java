/*******************************************************************************
 * This program and the accompanying materials
 * are made available under the terms of the Common Public License v1.0
 * which accompanies this distribution, and is available at 
 * http://www.eclipse.org/legal/cpl-v10.html
 * 
 * Contributors:
 *     Peter Smith
 *******************************************************************************/
package org.boris.expr;

import org.boris.expr.parser.ExprLexer;

public class ExprString extends Expr
{
    public final String str;

    public ExprString(String str) {
        super(ExprType.String, false);
        this.str = str;
    }

    public String toString() {
        return "\"" + ExprLexer.escapeJavaString(str) + "\"";
    }
}
