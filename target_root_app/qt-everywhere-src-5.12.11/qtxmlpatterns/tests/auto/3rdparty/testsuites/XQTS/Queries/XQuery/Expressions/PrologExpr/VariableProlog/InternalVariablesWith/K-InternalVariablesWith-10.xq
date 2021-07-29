(:*******************************************************:)
(: Test: K-InternalVariablesWith-10                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A namespace declaration must appear before a variable declaration, and its prefix is not inscope for the variable declaration. :)
(:*******************************************************:)
declare variable $prefix:var1 := 2;
declare namespace prefix = "http://example.com/myNamespace";
true()