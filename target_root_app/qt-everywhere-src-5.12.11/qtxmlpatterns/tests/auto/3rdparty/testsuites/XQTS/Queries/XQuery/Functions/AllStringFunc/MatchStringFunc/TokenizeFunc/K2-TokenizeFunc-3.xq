(:*******************************************************:)
(: Test: K2-TokenizeFunc-3                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: fn:tokenize with a positional predicate(#2). :)
(:*******************************************************:)
fn:tokenize(("abracadabra", current-time())[1] treat as xs:string,
             "(ab)|(a)")[last() - 1]