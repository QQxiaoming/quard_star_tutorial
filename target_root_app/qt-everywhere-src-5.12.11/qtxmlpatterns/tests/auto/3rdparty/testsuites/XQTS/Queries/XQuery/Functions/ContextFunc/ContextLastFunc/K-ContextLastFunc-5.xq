(:*******************************************************:)
(: Test: K-ContextLastFunc-5                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `(1, 2, 3, current-time())[last() - 1] treat as xs:integer eq 3`. :)
(:*******************************************************:)
(1, 2, 3, current-time())[last() - 1] treat as xs:integer eq 3