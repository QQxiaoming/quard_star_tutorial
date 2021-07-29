xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-007                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Use EQName in a decimal format                :)
(:*******************************************************:)

declare decimal format 'http://www.example.com/ns':format grouping-separator="'";

<a xmlns:ex="http://www.example.com/ns">{format-number(1e9, "#'###'###'##0.00", 'ex:format')}</a>