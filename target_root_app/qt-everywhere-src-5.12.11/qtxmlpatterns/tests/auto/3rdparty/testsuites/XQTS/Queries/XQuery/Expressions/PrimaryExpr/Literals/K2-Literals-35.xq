(:*******************************************************:)
(: Test: K2-Literals-35                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Attempt to evaluate a library module. The specification does not specify what an implementation should do, so we allow any output and any error code. :)
(:*******************************************************:)
module namespace prefix = "http://example.com";
declare function prefix:myFunction()
{
1
}; 