(:*******************************************************:)
(: Test: K2-Axes-52                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A type error in a complex path expression. Some implementations may optimize away the error. :)
(:*******************************************************:)
declare variable $myVar := <e/>;
empty($myVar/preceding-sibling::comment()/a/b/c/1/@*)