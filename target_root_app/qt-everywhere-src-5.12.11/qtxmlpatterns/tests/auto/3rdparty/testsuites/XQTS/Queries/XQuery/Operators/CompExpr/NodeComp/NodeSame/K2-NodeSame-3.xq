(:*******************************************************:)
(: Test: K2-NodeSame-3                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure node identity is is correct for variables and element constructors(#2). :)
(:*******************************************************:)
declare variable $var := <elem/>;
                        not($var is <elem/>)