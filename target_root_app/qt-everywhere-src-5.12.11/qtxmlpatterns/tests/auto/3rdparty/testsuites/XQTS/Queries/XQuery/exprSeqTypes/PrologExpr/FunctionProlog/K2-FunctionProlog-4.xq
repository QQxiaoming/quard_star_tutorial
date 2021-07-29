(:*******************************************************:)
(: Test: K2-FunctionProlog-4                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Have two function callsites as arguments to '!='. :)
(:*******************************************************:)
declare function local:myFunction($arg as xs:integer) as xs:integer
{
    ((if($arg eq 1)
    then 1
    else $arg - 1), current-time())[1] treat as xs:integer
};
not(local:myFunction(1) != local:myFunction(2))