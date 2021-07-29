(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-27                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: '*' is not allowed for the atomic type in element(). :)
(:*******************************************************:)
declare variable $v as element(notWildcard, xs:untyped*)+ := <e/>; 1