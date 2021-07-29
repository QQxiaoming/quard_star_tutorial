(:*******************************************************:)
(: Test: K2-CondExpr-3                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: It is ok to name a function 'unordered'.     :)
(:*******************************************************:)
declare default function namespace "http://www.example.com/";
declare function unordered()
{
1
};
unordered()