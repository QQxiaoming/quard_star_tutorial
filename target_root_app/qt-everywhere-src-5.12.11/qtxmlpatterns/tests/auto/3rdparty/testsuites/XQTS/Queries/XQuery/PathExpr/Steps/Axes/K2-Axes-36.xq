(:*******************************************************:)
(: Test: K2-Axes-36                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: '/' at the end of a path expression is a syntax error(#2). :)
(:*******************************************************:)
declare variable $myVar := <e/>;
            $myVar/(/)/