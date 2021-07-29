(:*******************************************************:)
(: Test: K2-InternalVariablesWith-1                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The query contains a type error despite the 'treat as' declaration. :)
(:*******************************************************:)
declare variable $var1 as xs:string := 1 treat as item();
            $var1