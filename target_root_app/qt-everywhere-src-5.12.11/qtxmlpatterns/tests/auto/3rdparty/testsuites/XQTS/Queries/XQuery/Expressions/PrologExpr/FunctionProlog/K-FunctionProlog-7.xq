(:*******************************************************:)
(: Test: K-FunctionProlog-7                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Arguments in functions cannot have default values initialized with '='(or in any other way). :)
(:*******************************************************:)
declare function local:myFunction($arg = 1 as xs:integer)
		{1};
		true()