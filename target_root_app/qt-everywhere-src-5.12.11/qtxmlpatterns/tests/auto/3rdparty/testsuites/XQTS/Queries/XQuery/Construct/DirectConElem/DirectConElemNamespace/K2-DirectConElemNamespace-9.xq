(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-9                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Default namespace declarations does not affect the default function namespace(#3). :)
(:*******************************************************:)
declare default function namespace "http://example.com";
<e xmlns="http://www.w3.org/2001/XMLSchema" a="{count()}"/>