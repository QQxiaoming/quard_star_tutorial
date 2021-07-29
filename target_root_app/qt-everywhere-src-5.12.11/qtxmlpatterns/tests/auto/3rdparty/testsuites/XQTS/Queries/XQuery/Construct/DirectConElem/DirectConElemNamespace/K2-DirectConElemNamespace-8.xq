(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-8                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Default namespace declarations does not affect the default function namespace(#2). :)
(:*******************************************************:)
declare default function namespace "http://example.com";
<e a="{p:count()}" xmlns:p="http://www.w3.org/2001/XMLSchema"/>