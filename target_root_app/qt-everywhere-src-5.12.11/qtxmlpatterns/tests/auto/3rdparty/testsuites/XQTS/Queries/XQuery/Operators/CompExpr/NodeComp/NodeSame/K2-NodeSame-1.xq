(:*******************************************************:)
(: Test: K2-NodeSame-1                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure node identity is is correct for variables and element constructors. :)
(:*******************************************************:)
declare variable $var := <elem/>;
                      <a>{$var}</a>/elem[1] is $var