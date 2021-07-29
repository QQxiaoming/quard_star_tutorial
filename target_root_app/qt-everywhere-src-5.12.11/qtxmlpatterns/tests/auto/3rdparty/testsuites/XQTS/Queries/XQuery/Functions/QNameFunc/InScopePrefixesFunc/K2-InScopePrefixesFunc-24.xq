(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-24                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure a prolog namespace declaration isn't in the in-scope bindings unless it's used. :)
(:*******************************************************:)
declare namespace p = "http://example.com/";
count(in-scope-prefixes(<element/>))