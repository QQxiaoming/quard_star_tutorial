(:*******************************************************:)
(: Test: K2-ReplaceFunc-8                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use fn:replace inside user function.         :)
(:*******************************************************:)
declare function local:doReplace($input as xs:string?, $pattern as xs:string, $replacement as xs:string) as xs:string
{
    fn:replace($input, $pattern, $replacement)
};
<result>
    <para>{fn:replace("ThiY Ybcd.", "Y", "Q")}</para>
    <para>{local:doReplace("ThiY iY a abYY.", "Y", "Q")}</para>
</result>,
fn:replace("ThiY abcdY.", "Y", "Q"),
local:doReplace("ThiY iY a abYY.", "Y", "Q")