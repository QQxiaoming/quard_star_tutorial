(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-17                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the namespace used in the name is part of the in-scope prefixes. :)
(:*******************************************************:)
declare namespace p = "http://example.com/";
count(in-scope-prefixes(<p:e/>)),
count(in-scope-prefixes(element p:e {()}))