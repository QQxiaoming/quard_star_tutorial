(:*******************************************************:)
(: Test: K2-SeqSubsequenceFunc-9                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Have subsequence inside a function body, to trap bugs related to inference, rewrites and function call sites(#2). :)
(:*******************************************************:)
declare function local:function()
{
(: This expression typically constant propagates to a sequence of integers. :)
subsequence((1, 2, 3), 1)
};
local:function()