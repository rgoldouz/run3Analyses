#include "Utils.h"

#include <cmath>
#include <limits>

bool solveNeutrinosChi2(
    const TLorentzVector& lep1,
    const TLorentzVector& lep2,		
    const TLorentzVector& jet1,
    const TLorentzVector& jet2,
    double METx, double METy,
    TLorentzVector& nu1_best,
    TLorentzVector& nu2_best
) {
    const double mtop = 172.5;
    const double sigma_top = 20.0;   // soft constraint
    const double beta = 1e-4;        // regularization

    // --- directions ---
    TVector3 dir1 = lep1.Vect().Unit();
    TVector3 dir2 = lep2.Vect().Unit();

    // --- adaptive lambda (based on Δφ) ---
    double dphi = fabs(TVector2::Phi_mpi_pi(jet1.Phi() - jet2.Phi()));
    double lambda = pow((M_PI - dphi)/M_PI, 2);

    // optional: scale with MET magnitude
    double MET = std::sqrt(METx*METx + METy*METy);
    lambda *= std::min(1.0, MET / 200.0);

    double bestChi2 = std::numeric_limits<double>::max();

    TLorentzVector nu1_tmp, nu2_tmp;

    // --- scan range based on jet pT ---
    double x1_max = jet1.Pt() * 2.0;
    double x2_max = jet2.Pt() * 2.0;

    // ---------- STEP 1: COARSE SCAN ----------
    double step_coarse = 20.0;

    double best_x1 = 0.0, best_x2 = 0.0;

    for (double x1 = 0.0; x1 < x1_max; x1 += step_coarse) {
        for (double x2 = 0.0; x2 < x2_max; x2 += step_coarse) {

            nu1_tmp.SetPxPyPzE(x1*dir1.X(), x1*dir1.Y(), x1*dir1.Z(), x1);
            nu2_tmp.SetPxPyPzE(x2*dir2.X(), x2*dir2.Y(), x2*dir2.Z(), x2);

            TLorentzVector top1 = jet1 + nu1_tmp;
            TLorentzVector top2 = jet2 + nu2_tmp;

            double m1 = top1.M();
            double m2 = top2.M();

            // MET term
            double dmetx = METx - (nu1_tmp.Px() + nu2_tmp.Px());
            double dmety = METy - (nu1_tmp.Py() + nu2_tmp.Py());
            double met_term = dmetx*dmetx + dmety*dmety;

            double chi2 =
                pow((m1 - mtop)/sigma_top, 2) +
                pow((m2 - mtop)/sigma_top, 2) +
                lambda * met_term +
                beta * (x1*x1 + x2*x2);

            if (chi2 < bestChi2) {
                bestChi2 = chi2;
                best_x1 = x1;
                best_x2 = x2;
            }
        }
    }

    // ---------- STEP 2: FINE SCAN ----------
    double step_fine = 5.0;
    double range = 40.0;

    for (double x1 = std::max(0.0, best_x1 - range);
         x1 < best_x1 + range; x1 += step_fine) {

        for (double x2 = std::max(0.0, best_x2 - range);
             x2 < best_x2 + range; x2 += step_fine) {

            nu1_tmp.SetPxPyPzE(x1*dir1.X(), x1*dir1.Y(), x1*dir1.Z(), x1);
            nu2_tmp.SetPxPyPzE(x2*dir2.X(), x2*dir2.Y(), x2*dir2.Z(), x2);

            TLorentzVector top1 = jet1 + nu1_tmp;
            TLorentzVector top2 = jet2 + nu2_tmp;

            double m1 = top1.M();
            double m2 = top2.M();

            double dmetx = METx - (nu1_tmp.Px() + nu2_tmp.Px());
            double dmety = METy - (nu1_tmp.Py() + nu2_tmp.Py());
            double met_term = dmetx*dmetx + dmety*dmety;

            double chi2 =
                pow((m1 - mtop)/sigma_top, 2) +
                pow((m2 - mtop)/sigma_top, 2) +
                lambda * met_term +
                beta * (x1*x1 + x2*x2);

            if (chi2 < bestChi2) {
                bestChi2 = chi2;

                nu1_best.SetPxPyPzE(x1*dir1.X(), x1*dir1.Y(), x1*dir1.Z(), x1);
                nu2_best.SetPxPyPzE(x2*dir2.X(), x2*dir2.Y(), x2*dir2.Z(), x2);
            }
        }
    }

    return (bestChi2 < 1e9);
}
/*
bool solveNeutrinosChi2(
    const TLorentzVector& lep1,
    const TLorentzVector& lep2,		
    const TLorentzVector& jet1,
    const TLorentzVector& jet2,
    double METx, double METy,
    TLorentzVector& nu1_best,
    TLorentzVector& nu2_best
) {
    const double mtop = 172.5;
    const double lambda = 1;  // tune this!
//    double dphi = fabs(deltaPhi(lep1.Phi(), lep2.Phi()));
//    double lambda = 200*pow((M_PI - dphi)/M_PI, 2);    

    TVector3 dir1 = lep1.Vect().Unit();
    TVector3 dir2 = lep2.Vect().Unit();

    double bestChi2 = std::numeric_limits<double>::max();

    // scan ranges (you can tune!)
    double x_min = 0.0;
    double x_max = 2000.0;  // GeV scale
    double step  = 10.0;
    double sigma_top = 20.0;
    for (double x1 = x_min; x1 < jet1.P()*2; x1 += step) {
        for (double x2 = x_min; x2 < jet2.P()*2; x2 += step) {

            // build neutrinos
            TLorentzVector nu1, nu2;

            nu1.SetPxPyPzE(x1*dir1.X(), x1*dir1.Y(), x1*dir1.Z(), x1);
            nu2.SetPxPyPzE(x2*dir2.X(), x2*dir2.Y(), x2*dir2.Z(), x2);

            // reconstruct tops
            TLorentzVector top1 = jet1 + nu1;
            TLorentzVector top2 = jet2 + nu2;

            double m1 = top1.M();
            double m2 = top2.M();


            // MET prediction
            double metx_pred = nu1.Px() + nu2.Px();
            double mety_pred = nu1.Py() + nu2.Py();

            double dmetx = METx - metx_pred;
            double dmety = METy - mety_pred;

            double met_term = dmetx*dmetx + dmety*dmety;

            // chi2
            double chi2 = pow((m1 - 172.5)/sigma_top, 2) +
    pow((m2 - 172.5)/sigma_top, 2) +
    lambda * met_term;

            if (chi2 < bestChi2) {
                bestChi2 = chi2;
                nu1_best = nu1;
                nu2_best = nu2;
            }
        }
    }
//            double metx_pred = nu1_best.Px() + nu2_best.Px();
//            double mety_pred = nu1_best.Py() + nu2_best.Py();

//            double dmetx = METx - metx_pred;
//            double dmety = METy - mety_pred;
//    if((jet1+jet2+nu1_best+nu2_best).M()>1500) cout<<(jet1+jet2+nu1_best+nu2_best).M()<<":"<<bestChi2<<":*****:"<<abs((jet1+nu1_best).M()-172)<<":"<<abs((jet2+nu2_best).M()-172)<<":"<<sqrt(dmetx*dmetx + dmety*dmety)<<":"<<lambda<<endl;
//    if((jet1+jet2+nu1_best+nu2_best).M()>1500)    cout<<"MET="<<sqrt(pow(METx,2)+pow(METy,2))<<":jetpt"<<jet1.Pt()<<":"<<jet2.Pt()<<endl;
//    if((jet1+jet2+nu1_best+nu2_best).M()>1500)    cout<<deltaPhi(jet1.Phi(), jet2.Phi())<<":"<<deltaPhi(lep1.Phi(), lep2.Phi())<<":"<<deltaPhi(nu1_best.Phi(),nu2_best.Phi())<<endl;
//    if((jet1+jet2+nu1_best+nu2_best).M()>1500)    cout<<nu1_best.Pt()<<":"<<nu1_best.Eta()<<":"<<nu1_best.Phi()<<":"<<nu2_best.Pt()<<":"<<nu2_best.Eta()<<":"<<nu2_best.Phi()<<endl;
    return (bestChi2 < 1e9);
}
*/


TLorentzVector solveBoostedNeutrinoForW(const TLorentzVector& pvis,
                                     const TVector3& jet_dir)
{
    const double MW = 80.2;
    TVector3 n = jet_dir.Unit();

    double dot = pvis.Px()*n.X() + pvis.Py()*n.Y() + pvis.Pz()*n.Z();

    double x = (MW*MW - pvis.M2()) / (2.0 * dot);

    if(x < 0) {
            x = 0;  // protect against unphysical solution
            cout<<"unphysical solution Mw= "<<pvis.M()<<endl;
    }

    TLorentzVector nu;

    double px = x * n.X();
    double py = x * n.Y();
    double pz = x * n.Z();
    double E  = sqrt(px*px + py*py + pz*pz);

    nu.SetPxPyPzE(px,py,pz,E);

    return nu;
}

TLorentzVector solveBoostedNeutrinoForTop(const TLorentzVector& pvis,
                                     const TVector3& jet_dir)
{
    const double MTOP = 172.5;
    TVector3 n = jet_dir.Unit();

    double dot = pvis.Px()*n.X() + pvis.Py()*n.Y() + pvis.Pz()*n.Z();

    double x = (MTOP*MTOP - pvis.M2()) / (2.0 * dot);

    if(x < 0) {
	    x = 0;  // protect against unphysical solution
	    cout<<"unphysical solution MTOP= "<<pvis.M()<<endl;
    }	    

    TLorentzVector nu;

    double px = x * n.X();
    double py = x * n.Y();
    double pz = x * n.Z();
    double E  = sqrt(px*px + py*py + pz*pz);

    nu.SetPxPyPzE(px,py,pz,E);

    return nu;
}

TLorentzVector solveResolvedNeutrino(const TLorentzVector& lep,
                                      double METx,
                                      double METy)
{
    const double MW   = 80.4;	
    double px = METx;
    double py = METy;

    double mu = (MW*MW)/2.0 + lep.Px()*px + lep.Py()*py;

    double a = lep.E()*lep.E() - lep.Pz()*lep.Pz();
    double b = -2.0 * mu * lep.Pz();
    double c = lep.E()*lep.E()*(px*px + py*py) - mu*mu;

    double discriminant = b*b - 4*a*c;

    double pz;

    if(discriminant >= 0)
    {
        double sol1 = (-b + sqrt(discriminant))/(2*a);
        double sol2 = (-b - sqrt(discriminant))/(2*a);

        if(fabs(sol1) < fabs(sol2))
            pz = sol1;
        else
            pz = sol2;
    }
    else
    {
        // complex solution → take real part
        pz = -b/(2*a);
    }

    double E = sqrt(px*px + py*py + pz*pz);

    TLorentzVector nu;
    nu.SetPxPyPzE(px,py,pz,E);

    return nu;
}


bool solveNeutrinosApprox(
    const TVector3& dir1,
    const TVector3& dir2,
    double METx, double METy,
    TLorentzVector& nu1,
    TLorentzVector& nu2
) {
    const double eps = 1e-6;

    // Matrix elements
    double a11 = dir1.X();
    double a12 = dir2.X();
    double a21 = dir1.Y();
    double a22 = dir2.Y();

    double det = a11*a22 - a12*a21;

    double x1 = 0.0, x2 = 0.0;

    if (fabs(det) > eps) {
        // --- Exact solution ---
        x1 = ( METx*a22 - METy*a12) / det;
        x2 = (-METx*a21 + METy*a11) / det;
    }

    // --- Enforce positivity ---
    x1 = std::max(0.0, x1);
    x2 = std::max(0.0, x2);

    // --- Build neutrinos ---
    nu1.SetPxPyPzE(
        x1 * dir1.X(),
        x1 * dir1.Y(),
        x1 * dir1.Z(),
        x1 // massless: E = |p|
    );

    nu2.SetPxPyPzE(
        x2 * dir2.X(),
        x2 * dir2.Y(),
        x2 * dir2.Z(),
        x2
    );

    // --- Optional: compute MET residual ---
    double recoMETx = nu1.Px() + nu2.Px();
    double recoMETy = nu1.Py() + nu2.Py();

    double dMETx = METx - recoMETx;
    double dMETy = METy - recoMETy;

    double residual = std::sqrt(dMETx*dMETx + dMETy*dMETy);

    // You can tune this cut
    if (residual > 100.0) {
        // very bad solution → reject if you want
        // return false;
    }

    return true;
}


bool reconstructNeutrinos(
    const TVector3& jet1_dir,
    const TVector3& jet2_dir,
    double METx,
    double METy,
    TLorentzVector& nu1,
    TLorentzVector& nu2)
{

    // Unit vectors along the jets
    TVector3 n1 = jet1_dir.Unit();
    TVector3 n2 = jet2_dir.Unit();

    double n1x = n1.X();
    double n1y = n1.Y();

    double n2x = n2.X();
    double n2y = n2.Y();

    // Solve the linear system
    double det = n1x*n2y - n1y*n2x;

    if(fabs(det) < 1e-6){
        // directions nearly parallel → no stable solution
        return false;
    }

    double x1 = ( METx*n2y - METy*n2x ) / det;
    double x2 = (-METx*n1y + METy*n1x ) / det;

    if(x1 < 0 || x2 < 0){
    // fallback solution
      x1 = METx*n1x + METy*n1y;
      x2 = METx*n2x + METy*n2y;
  
      x1 = std::max(0.0, x1);
      x2 = std::max(0.0, x2);	    
    }

    // Neutrino 3-momenta
    TVector3 pnu1 = x1 * n1;
    TVector3 pnu2 = x2 * n2;

    // Massless neutrino energy
    double E1 = pnu1.Mag();
    double E2 = pnu2.Mag();

    nu1.SetPxPyPzE(pnu1.X(), pnu1.Y(), pnu1.Z(), E1);
    nu2.SetPxPyPzE(pnu2.X(), pnu2.Y(), pnu2.Z(), E2);

    return true;
}

int getIndex(std::vector<TString> words, std::string target) {
    auto it = std::find(words.begin(), words.end(), target);
    int index=0;
    if (it != words.end()) {
        index = std::distance(words.begin(), it);
    } else {
        std::cout << "Not found" << std::endl;
    }
    return index;
}

int getVecPos(std::vector<TString> vec, string element){
    int i;
    for(i = 0; i < vec.size(); i++){
      if(vec[i] == element) break;
    }
    if(i == vec.size()){
        std::cout<<"No such element as "<<element<<" found. Please enter again: ";
        std::cin>>element;
        i = getVecPos(vec, element);
    }
    return i;
}

double dR(double eta1, double phi1, double eta2, double phi2){
    double dphi = phi2 - phi1;
    double deta = eta2 - eta1;
    static const double pi = TMath::Pi();
    dphi = TMath::Abs( TMath::Abs(dphi) - pi ) - pi;
    return TMath::Sqrt( dphi*dphi + deta*deta );
}

void displayProgress(long current, long max){
  using std::cerr;
  if (max<1000) return;
  if (current%(max/100)!=0 && current<max-1) return;

  int width = 52; // Hope the terminal is at least that wide.
  int barWidth = width - 2;
  cerr << "\x1B[2K"; // Clear line
  cerr << "\x1B[2000D"; // Cursor left
  cerr << '[';
  for(int i=0 ; i<barWidth ; ++i){ if(i<barWidth*current/max){ cerr << '=' ; }else{ cerr << ' ' ; } }
  cerr << ']';
  cerr << " " << Form("%8d/%8d (%5.2f%%)", (int)current, (int)max, 100.0*current/max) ;
  cerr.flush();
}

Double_t deltaPhi(Double_t phi1, Double_t phi2) {
  Double_t dPhi = phi1 - phi2;
  if (dPhi > TMath::Pi()) dPhi -= 2.*TMath::Pi();
  if (dPhi < -TMath::Pi()) dPhi += 2.*TMath::Pi();
  return dPhi;
}


Double_t deltaR(Double_t eta1, Double_t phi1, Double_t eta2, Double_t phi2) {
  Double_t dEta, dPhi ;
  dEta = eta1 - eta2;
  dPhi = deltaPhi(phi1, phi2);
  return sqrt(dEta*dEta+dPhi*dPhi);
}

int signnum_typical(double x) {
  if (x > 0.0) return 1;
  if (x < 0.0) return -1;
  return 0;
}

bool ComparePtLep(lepton_candidate *a, lepton_candidate *b) { return a->pt_ > b->pt_; }
bool ComparePtJet(jet_candidate *a, jet_candidate *b) { return a->pt_ > b->pt_; }
bool CompareMassJet(jet_candidate *a, jet_candidate *b) { return a->mass_ > b->mass_; }
std::vector<bool> parsePhotonVIDCuts(int bitMap, int cutLevel){
    //    *         | Int_t VID compressed bitmap (MinPtCut,PhoSCEtaMultiRangeCut,PhoSingleTowerHadOverEmCut,PhoFull5x5SigmaIEtaIEtaCut,PhoAnyPFIsoWithEACut,PhoAnyPFIsoWithEAAndQuadScalingCut,PhoAnyPFIsoWithEACut), 2 bits per cut*

    bool passHoverE  = (bitMap>>4&3)  >= cutLevel;
    bool passSIEIE   = (bitMap>>6&3)  >= cutLevel;
    bool passChIso   = (bitMap>>8&3)  >= cutLevel;
    bool passNeuIso  = (bitMap>>10&3) >= cutLevel;
    bool passPhoIso  = (bitMap>>12&3) >= cutLevel;


    bool passID = passHoverE && passSIEIE && passChIso && passNeuIso && passPhoIso;

    std::vector<bool> cuts;
    cuts.push_back(passID);
    cuts.push_back(passHoverE);
    cuts.push_back(passSIEIE);
    cuts.push_back(passChIso);
    cuts.push_back(passNeuIso);
    cuts.push_back(passPhoIso);

    return cuts;

}

/*
float scale_factor( TH2F* h, float X, float Y , TString uncert){
  int NbinsX=h->GetXaxis()->GetNbins();
  int NbinsY=h->GetYaxis()->GetNbins();
  float x_min=h->GetXaxis()->GetBinLowEdge(1);
  float x_max=h->GetXaxis()->GetBinLowEdge(NbinsX)+h->GetXaxis()->GetBinWidth(NbinsX);
  float y_min=h->GetYaxis()->GetBinLowEdge(1);
  float y_max=h->GetYaxis()->GetBinLowEdge(NbinsY)+h->GetYaxis()->GetBinWidth(NbinsY);
  TAxis *Xaxis = h->GetXaxis();
  TAxis *Yaxis = h->GetYaxis();
  Int_t binx=1;
  Int_t biny=1;
  if(x_min < X && X < x_max) binx = Xaxis->FindBin(X);
  else binx= (X<=x_min) ? 1 : NbinsX ;
  if(y_min < Y && Y < y_max) biny = Yaxis->FindBin(Y);
  else biny= (Y<=y_min) ? 1 : NbinsY ;
  if(uncert=="up") return (h->GetBinContent(binx, biny)+h->GetBinError(binx, biny));
  else if(uncert=="down") return (h->GetBinContent(binx, biny)-h->GetBinError(binx, biny));
  else return  h->GetBinContent(binx, biny);
}
*/

float scale_factor( TH2F* h, float X, float Y , TString uncert, bool eff=false, bool out=false){
  int NbinsX=h->GetXaxis()->GetNbins();
  int NbinsY=h->GetYaxis()->GetNbins();
  float x_min=h->GetXaxis()->GetBinLowEdge(1);
  float x_max=h->GetXaxis()->GetBinLowEdge(NbinsX)+h->GetXaxis()->GetBinWidth(NbinsX);
  float y_min=h->GetYaxis()->GetBinLowEdge(1);
  float y_max=h->GetYaxis()->GetBinLowEdge(NbinsY)+h->GetYaxis()->GetBinWidth(NbinsY);
  TAxis *Xaxis = h->GetXaxis();
  TAxis *Yaxis = h->GetYaxis();
  Int_t binx=1;
  Int_t biny=1;
  if(x_min < X && X < x_max) binx = Xaxis->FindBin(X);
  else binx= (X<=x_min) ? 1 : NbinsX ;
  if(y_min < Y && Y < y_max) biny = Yaxis->FindBin(Y);
  else biny= (Y<=y_min) ? 1 : NbinsY ;
  if(uncert=="up") return (h->GetBinContent(binx, biny)+h->GetBinError(binx, biny));
  if(uncert=="down") return (h->GetBinContent(binx, biny)-h->GetBinError(binx, biny));
  if(uncert=="central") return  h->GetBinContent(binx, biny);
}

float topPt(float pt){
  return (0.973 - (0.000134 * pt) + (0.103 * exp(pt * (-0.0118))));
}

TLorentzVector Wneutrino(double MET, double METphi, double leptonPT, double leptonEta, double leptonPhi) {
  double mW=80.4;
  double neutrinoPX=0;
  double neutrinoPY=0;
  double neutrinoPZ=0;
  TLorentzVector lepton;
  lepton.SetPtEtaPhiM(leptonPT, leptonEta, leptonPhi, 0);
  double leptonPZ=lepton.Pz();
  double mu=(std::pow(mW,2)/2)+std::cos(deltaPhi(METphi,leptonPhi))*MET*leptonPT;
  double determinant = (std::pow(mu,2)*std::pow(leptonPZ,2)/std::pow(leptonPT,4))-(std::pow(MET,2)*(std::pow(leptonPT,2)+std::pow(leptonPZ,2))-std::pow(mu,2))/std::pow(leptonPT,2);
  if (determinant<0){
    MET=(1.+std::cos(deltaPhi(METphi,leptonPhi)))*std::pow(mW,2)/(2*leptonPT*std::pow(std::sin(deltaPhi(METphi,leptonPhi)),2));
    mu=(std::pow(mW,2)/2)+std::cos(deltaPhi(METphi,leptonPhi))*MET*leptonPT;
    determinant=0.;
  }
  double neutrinoPZplus=(mu*leptonPZ/std::pow(leptonPT,2))+std::sqrt(determinant);
  double neutrinoPZminus=(mu*leptonPZ/std::pow(leptonPT,2))-std::sqrt(determinant);
  neutrinoPZ=neutrinoPZminus;
  if(std::fabs(neutrinoPZplus)<std::fabs(neutrinoPZminus)){
    neutrinoPZ=neutrinoPZplus;
  }
  neutrinoPX=MET*std::cos(METphi);
  neutrinoPY=MET*std::sin(METphi);
  TLorentzVector neutrino;
  neutrino.SetPxPyPzE(neutrinoPX,neutrinoPY,neutrinoPZ,std::sqrt(std::pow(neutrinoPX,2)+std::pow(neutrinoPY,2)+std::pow(neutrinoPZ,2)));
  return  neutrino;
}

int vInd(std::map<TString, std::tuple<int, int, const double*>> V, TString name){
  return std::get<0>(V.find(name)->second);
}

