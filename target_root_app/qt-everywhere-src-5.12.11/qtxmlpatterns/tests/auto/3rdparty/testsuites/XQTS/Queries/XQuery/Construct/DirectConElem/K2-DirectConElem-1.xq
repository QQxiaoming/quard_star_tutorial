(:*******************************************************:)
(: Test: K2-DirectConElem-1                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure processing-instructions aren't included when extracting the string-value from elements. :)
(:*******************************************************:)
string(<pi>{<?pi x?>}</pi>) eq ""