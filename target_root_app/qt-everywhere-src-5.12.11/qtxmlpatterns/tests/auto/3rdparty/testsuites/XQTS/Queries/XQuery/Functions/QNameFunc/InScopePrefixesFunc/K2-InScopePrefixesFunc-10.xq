(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-10                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that a default namespace declaration is picked up. :)
(:*******************************************************:)
declare default element namespace "http://www.example.com";
count(fn:in-scope-prefixes(<e/>))