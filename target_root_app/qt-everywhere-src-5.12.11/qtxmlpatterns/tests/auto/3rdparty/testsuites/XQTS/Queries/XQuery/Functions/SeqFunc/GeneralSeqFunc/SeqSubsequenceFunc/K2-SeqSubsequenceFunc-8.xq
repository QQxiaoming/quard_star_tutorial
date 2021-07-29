(:*******************************************************:)
(: Test: K2-SeqSubsequenceFunc-8                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Have subsequence inside a function body, to trap bugs related to inference, rewrites and function call sites. :)
(:*******************************************************:)
declare function local:function()
{
(: This expression typically constant propagates to a sequence of integers. :)
subsequence(subsequence((1, 2, 3, 4), 3, 1), 1, 4)
};
local:function()