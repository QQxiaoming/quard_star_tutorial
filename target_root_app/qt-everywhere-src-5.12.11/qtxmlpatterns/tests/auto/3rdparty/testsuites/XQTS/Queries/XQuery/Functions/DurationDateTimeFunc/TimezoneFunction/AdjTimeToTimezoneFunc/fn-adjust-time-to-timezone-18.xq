(:*******************************************************:)
(:Test: adjust-time-to-timezone-18                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The "adjust-time-to-timezone" function :)
(:as part of a subtraction expression, which results on a negative number. :)
(:Uses one adjust-time-to-timezone function and one      :)
(:xs:time constructor. Use zulu timezone and empty sequence for 2nd :)
(: argument.                                             :)
(:*******************************************************:)

let $tz := xs:dayTimeDuration("PT10H")
return
fn:adjust-time-to-timezone(xs:time("01:00:00Z"),$tz) - xs:time("10:00:00Z")