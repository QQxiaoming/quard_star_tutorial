(:*******************************************************:)
(:Test: op-dateTime-greater-than-6                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-greater-than" function that  :)
(:return false and used together with fn:not(ge operator):)
(:*******************************************************:)
 
fn:not(xs:dateTime("2002-04-02T12:00:00Z") ge xs:dateTime("2008-04-02T12:00:00Z"))