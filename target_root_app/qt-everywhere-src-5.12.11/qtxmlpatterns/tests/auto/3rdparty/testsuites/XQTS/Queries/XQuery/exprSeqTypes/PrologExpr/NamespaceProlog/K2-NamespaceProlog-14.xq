(:*******************************************************:)
(: Test: K2-NamespaceProlog-14                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Namespace declaration must appear before a variable declaration. :)
(:*******************************************************:)
declare variable $inputDoc := 2;
declare namespace x = "http://example.com/";
1