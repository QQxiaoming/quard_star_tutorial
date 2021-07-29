(:*******************************************************:)
(:Test: timezone-from-date-12                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-date" function   :)
(:as an argument to the fn:number function.  returns "NaN".:) 
(:*******************************************************:)

fn:number(fn:timezone-from-date(xs:date("1970-01-02+10:00")))
