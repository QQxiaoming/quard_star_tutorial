(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-12                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A name test whose namespace is declared with a default namespace attribute(#2). :)
(:*******************************************************:)
declare default function namespace "http://example.com";
<e a="{nametest}" xmlns="http://www.w3.org/2001/XMLSchema"/>