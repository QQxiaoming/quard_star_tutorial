(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-14                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A name test whose namespace is declared with a prefixed namespace attribute(#2). :)
(:*******************************************************:)
declare default function namespace "http://example.com";
<e p:p="{p:nametest}" xmlns:p="http://www.w3.org/2001/XMLSchema"/>