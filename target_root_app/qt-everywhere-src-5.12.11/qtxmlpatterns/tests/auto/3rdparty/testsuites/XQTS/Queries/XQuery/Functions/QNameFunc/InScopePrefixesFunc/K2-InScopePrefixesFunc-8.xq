(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-8                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the in-scope prefixes dealt with correctly for double default namespace declarations of different kinds. :)
(:*******************************************************:)
declare default element namespace "http://www.example.com/A";
          count(in-scope-prefixes(<anElement xmlns="http://www.example.com/B"/>))