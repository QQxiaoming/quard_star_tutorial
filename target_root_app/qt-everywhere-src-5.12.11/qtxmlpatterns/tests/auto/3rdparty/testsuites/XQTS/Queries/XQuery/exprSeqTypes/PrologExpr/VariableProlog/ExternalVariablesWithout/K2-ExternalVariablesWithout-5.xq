(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-5                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: If the value for an external variable can't be supplied, XPDY0002 must be raised(#2). :)
(:*******************************************************:)
declare namespace e = "http://example.com/ANamespace";
declare variable $e:exampleComThisVarIsNotRecognized external;
$e:exampleComThisVarIsNotRecognized