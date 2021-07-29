(:*******************************************************:)
(: Test: K2-NamespaceURIFromQNameFunc-1                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that xmlns declarations in direct element constructors are honored in embedded expressions. :)
(:*******************************************************:)
declare namespace ns = "http://example.com/ANamespace";
string(<name xmlns:ns="http://example.com/BNamespace">{namespace-uri-from-QName("ns:foo" cast as xs:QName)}</name>)