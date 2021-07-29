(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-7                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An implementation may raise XPDY0002 on a declared external variable, although not necessary since it isn't used(#2). :)
(:*******************************************************:)
declare namespace e = "http://example.com/ANamespace";
declare variable $e:exampleComThisVarIsNotRecognized external;
1