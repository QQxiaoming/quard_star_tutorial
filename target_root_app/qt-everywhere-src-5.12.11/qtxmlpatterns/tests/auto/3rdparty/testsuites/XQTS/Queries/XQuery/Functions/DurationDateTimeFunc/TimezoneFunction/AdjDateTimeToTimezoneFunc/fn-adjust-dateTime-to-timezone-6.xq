(:*******************************************************:)
(:Test: adjust-dateTime-to-timezone-6                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The "adjust-dateTime-to-timezone" function   :)
(:as per example 6 (for this function) of the F&O  specs. :)
(:*******************************************************:)

fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T00:00:00+01:00"), xs:dayTimeDuration("-PT8H"))